/*
 * =====================================================================================
 *
 *       Filename: creaturemovable.cpp
 *        Created: 04/25/2020 22:25:23
 *    Description: 
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

#include <vector>
#include "log.hpp"
#include "mathf.hpp"
#include "processrun.hpp"
#include "creaturemovable.hpp"

extern Log *g_log;

std::vector<PathFind::PathNode> CreatureMovable::parseMovePath(int x0, int y0, int x1, int y1, bool checkGround, int checkCreature)
{
    if(!m_processRun->CanMove(true, 0, x0, y0)){
        return {};
    }

    auto nMaxStep = maxStep();
    switch(auto nLDistance2 = mathf::LDistance2(x0, y0, x1, y1)){
        case 0:
            {
                return {{x0, y0}};
            }
        case 1:
        case 2:
            {
                // dst is at one-hop distance
                // so there couldn't be any middle grids blocking

                if(checkGround){
                    if(m_processRun->CanMove(true, 0, x1, y1)){
                        // we ignore checkCreature
                        // because this always gives the best path
                        return {{x0, y0}, {x1, y1}};
                    }else{
                        // can't find a path to dst
                        // return the starting node, return empty means errors
                        return {{x0, y0}};
                    }
                }else{
                    // won't check ground
                    // then directly return the unique path
                    return {{x0, y0}, {x1, y1}};
                }
            }
        default:
            {
                // 1. one hop distance
                // 2. more complex distance

                if(false
                        || nLDistance2 == nMaxStep * nMaxStep
                        || nLDistance2 == nMaxStep * nMaxStep * 2){

                    // one hop distance
                    // but not with distance = 1 or 2
                    // there could be middle grid blocking this hop

                    if(m_processRun->CanMove(true, 0, x0, y0, x1, y1)){
                        switch(checkCreature){
                            case 0:
                            case 1:
                                {
                                    // not check creatures
                                    // and we can reach dst in one-hop
                                    return {{x0, y0}, {x1, y1}};
                                }
                            case 2:
                                {
                                    int nDX = (x1 > x0) - (x1 < x0);
                                    int nDY = (y1 > y0) - (y1 < y0);

                                    // we need to avoid check the first node
                                    // since it will fail by occupation of itself

                                    if(m_processRun->CanMove(true, 2, x0 + nDX, y0 + nDY, x1, y1)){
                                        // we are checking the creatures
                                        // and no creaturs standing on the one-hop path
                                        return {{x0, y0}, {x1, y1}};
                                    }

                                    // can reach in one hop but there is creatures on the path
                                    // and we can't ignore the creatures
                                    // leave it to the complex path solver
                                    break;
                                }
                            default:
                                {
                                    throw fflerror("invalid CheckCreature provided: %d, should be (0, 1, 2)", checkCreature);
                                }
                        }

                    }else{
                        // can't reach in one hop
                        // means there is middle grids blocking this path
                        // leave it to the complex path solver
                    }

                }else{

                    // not one-hop distance
                    // leave it to the complex path solver
                }

                // the complex path solver
                // we can always use this solver only

                ClientPathFinder stPathFinder(checkGround, checkCreature, nMaxStep);
                if(stPathFinder.Search(x0, y0, x1, y1)){
                    return stPathFinder.GetPathNode();
                }else{
                    // we can't find a path
                    // return the starting point only
                    return {{x0, y0}};
                }
            }
    }
}

std::deque<MotionNode> CreatureMovable::makeWalkMotionQueue(int startX, int startY, int endX, int endY, int speed)
{
    if(mathf::LDistance2(startX, startY, endX, endY) == 0){
        return {};
    }

    const auto pathNodes = parseMovePath(startX, startY, endX, endY, true, 1);
    switch(pathNodes.size()){
        case 0:
        case 1:
            {
                // 0 means error
                // 1 means can't find a path here since we know LDistance2 != 0
                throw fflerror("Can't find a path: (%d, %d) -> (%d, %d)", startX, startY, endX, endY);
            }
        default:
            {
                // we get a path
                // make a motion list for the path

                std::deque<MotionNode> motionQueue;
                for(size_t nIndex = 1; nIndex < pathNodes.size(); ++nIndex){
                    const auto x0 = pathNodes[nIndex - 1].X;
                    const auto y0 = pathNodes[nIndex - 1].Y;
                    const auto x1 = pathNodes[nIndex    ].X;
                    const auto y1 = pathNodes[nIndex    ].Y;

                    if(const auto motionNode = makeWalkMotion(x0, y0, x1, y1, speed)){
                        motionQueue.push_back(motionNode);
                    }
                    else{
                        throw fflerror("Can't make a motioni node: (%d, %d) -> (%d, %d)", x0, y0, x1, y1);
                    }
                }
                return motionQueue;
            }
    }
}

bool CreatureMovable::motionQueueValid() const
{
    if(m_motionQueue.empty()){
        return true;
    }

    const MotionNode *lastMotionPtr = &m_currMotion;
    for(const auto &motion: m_motionQueue){
        if(motionValid(motion)
                && (lastMotionPtr->endX == motion.x)
                && (lastMotionPtr->endY == motion.y)){
            lastMotionPtr = &motion;
        }
        else{
            g_log->addLog(LOGTYPE_WARNING, "Invalid motion queue:");
            m_currMotion.print();
            for(auto &node: m_motionQueue){
                node.print();
            }
            return false;
        }
    }
    return true;
}

bool CreatureMovable::moveNextMotion()
{
    if(!m_forceMotionQueue.empty()){
        m_currMotion = std::move(m_forceMotionQueue.front());
        m_forceMotionQueue.pop_front();
        return true;
    }

    if(m_motionQueue.empty()){
        m_currMotion = makeIdleMotion();
        return true;
    }

    if(motionQueueValid()){
        m_currMotion = std::move(m_motionQueue.front());
        m_motionQueue.pop_front();
        return true;
    }

    g_log->addLog(LOGTYPE_WARNING, "Motion queue invalid, reset idle state");
    m_motionQueue.clear();
    m_currMotion = makeIdleMotion();
    return false;
}

std::tuple<int, int> CreatureMovable::getShift() const
{
    switch(m_currMotion.motion){
        case MOTION_WALK:           // human
        case MOTION_RUN:            // human
        case MOTION_ONHORSEWALK:    // human
        case MOTION_ONHORSERUN:     // human
        case MOTION_MON_WALK:       // monster
            {
                break;
            }
        default:
            {
                return {0, 0};
            }
    }

    const auto currStepCount = currStep();
    switch(currStepCount){
        case 1:
        case 2:
        case 3:
            {
                break;
            }
        default:
            {
                throw fflerror("invalid step count: %d", currStepCount);
            }
    }

    const auto frameCount = motionFrameCount(m_currMotion.motion, m_currMotion.direction);
    if(frameCount <= 0){
        throw fflerror("invalid frame count: %d", frameCount);
    }

    if(m_currMotion.frame >= frameCount){
        throw fflerror("invalid frame: %d", m_currMotion.frame);
    }

    const float shiftDX = 1.0f * SYS_MAPGRIDXP * currStepCount / frameCount;
    const float shiftDY = 1.0f * SYS_MAPGRIDYP * currStepCount / frameCount;

    const int shiftX = (int)(std::lround(shiftDX * (m_currMotion.frame + 1)));
    const int shiftY = (int)(std::lround(shiftDY * (m_currMotion.frame + 1)));

    switch(m_currMotion.direction){
        case DIR_UP       : return {      0, -shiftY};
        case DIR_UPRIGHT  : return { shiftX, -shiftY};
        case DIR_RIGHT    : return { shiftX,       0};
        case DIR_DOWNRIGHT: return { shiftX,  shiftY};
        case DIR_DOWN     : return {      0,  shiftY};
        case DIR_DOWNLEFT : return {-shiftX,  shiftY};
        case DIR_LEFT     : return {-shiftX,       0};
        case DIR_UPLEFT   : return {-shiftX, -shiftY};
        default           : return {      0,       0};
    }
}
