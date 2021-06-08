/*
 * =====================================================================================
 *
 *       Filename: wilimagepackage.hpp
 *        Created: 02/14/2016 16:33:12
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

#pragma once

#include <vector>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include "fflerror.hpp"

#pragma pack(push, 1)

struct WILFILEHEADER
{
    int16_t     shComp;
    char        szTitle[20];
    int16_t     shVer;
    int32_t     nImageCount;
};

struct WIXIMAGEINFO
{
    char        szTitle[20];
    int32_t     nIndexCount;
};

struct WILIMAGEINFO
{
    int16_t     shWidth;
    int16_t     shHeight;
    int16_t     shPX;
    int16_t     shPY;
    char        bShadow;                    
    int16_t     shShadowPX;
    int16_t     shShadowPY;
    uint32_t    dwImageLength;
};

#pragma pack(pop)

class WilImagePackage
{
    private:
        WIXIMAGEINFO  m_wixImageInfo;
        WILIMAGEINFO  m_currentWilImageInfo;
        WILFILEHEADER m_wilFileHeader;

    private:
        int32_t m_currentImageIndex;
        bool    m_currentImageValid;

    private:
        std::vector<uint16_t> m_currentImageBuffer;
        std::vector< int32_t> m_wilPosition;

    private:
        FILE *m_wilFile;

    public:
        WilImagePackage();
        ~WilImagePackage();

    public:
        int32_t ImageCount();
        int32_t IndexCount();

    public:
        int16_t Version();

    public:
        bool SetIndex(uint32_t);

    public:
        bool Load(const char *, const char *, const char *);
        void Decode(uint32_t *, uint32_t, uint32_t, uint32_t);

    public:
        const WILFILEHEADER &HeaderInfo() const;

    public:
        const WILIMAGEINFO  &CurrentImageInfo();
        bool                 CurrentImageValid();
        const uint16_t      *CurrentImageBuffer();

    private:
        static int wixOff(int version)
        {
            switch(version){
                case   17: return 24;
                case 5000: return 28;
                case 6000: return 32;
                default  : throw bad_reach();
            }
        }

        static int wilOff(int version)
        {
            switch(version){
                case   17: return 17;
                case 5000: return 21;
                case 6000: return 21;
                default  : throw bad_reach();
            }
        }
};
