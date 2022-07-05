//unit_test.cpp
// for unitTest
//
/*
 The MIT License (MIT)
 Copyright (c) 2012-2022 HouSisong

 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following
 conditions:

 The above copyright notice and this permission notice shall be
 included in all copies of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
*/

#include <iostream>
#include <string>
#include <vector>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>//for ptrdiff_t
#include <stdlib.h>
#include "math.h"
#include "../libHDiffPatch/HDiff/diff.h"
#include "../libHDiffPatch/HDiff/diff_for_hpatch_lite.h"
#include "../libHDiffPatch/HPatch/patch.h"
#include "../libHDiffPatch/HPatchLite/hpatch_lite.h"
#include "../libHDiffPatch/HDiff/private_diff/limit_mem_diff/stream_serialize.h"
using namespace hdiff_private;
typedef unsigned char   TByte;
typedef ptrdiff_t       TInt;
typedef size_t          TUInt;
const long kRandTestCount=20000;
//#define _AttackPacth_ON

//===== select compress plugin =====
#define _CompressPlugin_no
//#define _CompressPlugin_zlib
//#define _CompressPlugin_bz2
//#define _CompressPlugin_lzma
//#define _CompressPlugin_lzma2
//#define _CompressPlugin_lz4
//#define _CompressPlugin_lz4hc
//#define _CompressPlugin_zstd
//#define _CompressPlugin_brotli
//#define _CompressPlugin_lzham
//#define _CompressPlugin_tuz

#define IS_NOTICE_compress_canceled 0 //for test, close compress fail notice
#define IS_REUSE_compress_handle    1 //for test, must in single thread
#include "../compress_plugin_demo.h"
#include "../decompress_plugin_demo.h"


#ifdef  _CompressPlugin_no
    const hdiff_TCompress* compressPlugin=0;
    hpatch_TDecompress* decompressPlugin=0;
    hpi_compressType compressHpiType=hpi_compressType_no;
#endif
#ifdef  _CompressPlugin_zlib
    const hdiff_TCompress* compressPlugin=&zlibCompressPlugin.base;
    hpatch_TDecompress* decompressPlugin=&zlibDecompressPlugin;
    hpi_compressType compressHpiType=hpi_compressType_zlib;
#endif
#ifdef  _CompressPlugin_bz2
    const hdiff_TCompress* compressPlugin=&bz2CompressPlugin.base;
    hpatch_TDecompress* decompressPlugin=&bz2DecompressPlugin;
    hpi_compressType compressHpiType=hpi_compressType_bz2;
#endif
#ifdef  _CompressPlugin_lzma
    const hdiff_TCompress* compressPlugin=&lzmaCompressPlugin.base;
    hpatch_TDecompress* decompressPlugin=&lzmaDecompressPlugin;
    hpi_compressType compressHpiType=hpi_compressType_lzma;
#endif
#ifdef  _CompressPlugin_lzma2
    const hdiff_TCompress* compressPlugin=&lzma2CompressPlugin.base;
    hpatch_TDecompress* decompressPlugin=&lzma2DecompressPlugin;
    hpi_compressType compressHpiType=hpi_compressType_lzma2;
#endif
#ifdef  _CompressPlugin_lz4
    const hdiff_TCompress* compressPlugin=&lz4CompressPlugin.base;
    hpatch_TDecompress* decompressPlugin=&lz4DecompressPlugin;
    hpi_compressType compressHpiType=hpi_compressType_lz4;
#endif
#ifdef  _CompressPlugin_lz4hc
    const hdiff_TCompress* compressPlugin=&lz4hcCompressPlugin.base;
    hpatch_TDecompress* decompressPlugin=&lz4DecompressPlugin;
    hpi_compressType compressHpiType=hpi_compressType_lz4;
#endif
#ifdef  _CompressPlugin_zstd
    const hdiff_TCompress* compressPlugin=&zstdCompressPlugin.base;
    hpatch_TDecompress* decompressPlugin=&zstdDecompressPlugin;
    hpi_compressType compressHpiType=hpi_compressType_zstd;
#endif
#ifdef  _CompressPlugin_brotli
    const hdiff_TCompress* compressPlugin=&brotliCompressPlugin.base;
    hpatch_TDecompress* decompressPlugin=&brotliDecompressPlugin;
    hpi_compressType compressHpiType=hpi_compressType_brotli;
#endif
#ifdef  _CompressPlugin_lzham
    const hdiff_TCompress* compressPlugin=&lzhamCompressPlugin.base;
    hpatch_TDecompress* decompressPlugin=&lzhamDecompressPlugin;
    hpi_compressType compressHpiType=hpi_compressType_lzham;
#endif
#ifdef  _CompressPlugin_tuz
    const hdiff_TCompress* compressPlugin=&tuzCompressPlugin.base;
    hpatch_TDecompress* decompressPlugin=&tuzDecompressPlugin;
    hpi_compressType compressHpiType=hpi_compressType_tuz;
#endif

int testCompress(const char* str,const char* error_tag){
    assert(  ((compressPlugin==0)&&(decompressPlugin==0))
           ||((compressPlugin!=0)&&(decompressPlugin!=0)));
    if (compressPlugin==0) return 0;
    assert(decompressPlugin->is_can_open(compressPlugin->compressType()));
    
    const TByte* data=(const TByte*)str;
    const size_t dataSize=strlen(str);
    std::vector<TByte> code((size_t)compressPlugin->maxCompressedSize(dataSize));
    size_t codeSize=hdiff_compress_mem(compressPlugin,code.data(),code.data()+code.size(),
                                       data,data+dataSize);
    if (codeSize>code.size()) {
        printf("\n testCompress compress error!!! tag:%s\n",error_tag); return 1; }
    code.resize(codeSize);
    
    std::vector<TByte> undata(dataSize);
    if (!hpatch_deccompress_mem(decompressPlugin,code.data(),code.data()+code.size(),
                                undata.data(),undata.data()+undata.size()))  {
        printf("\n testCompress decompress error!!! tag:%s\n",error_tag); return 1; }
    if (0!=memcmp(str,undata.data(),undata.size()))  {
        printf("\n testCompress decompress data error!!! tag:%s\n",error_tag); return 1; }
    return 0;
}
    

static bool _patch_mem_stream(TByte* newData,TByte* newData_end,
                              const TByte* oldData,const TByte* oldData_end,
                              const TByte* diff,const TByte* diff_end){
    struct hpatch_TStreamOutput out_newStream;
    struct hpatch_TStreamInput  oldStream;
    struct hpatch_TStreamInput  diffStream;
    mem_as_hStreamOutput(&out_newStream,newData,newData_end);
    mem_as_hStreamInput(&oldStream,oldData,oldData_end);
    mem_as_hStreamInput(&diffStream,diff,diff_end);
    
    return 0!=patch_stream(&out_newStream,&oldStream,&diffStream);
}

static bool check_diff_stream(const TByte* newData,const TByte* newData_end,
                              const TByte* oldData,const TByte* oldData_end,
                              const TByte* diff,const TByte* diff_end){
    std::vector<TByte> testNewData(newData_end-newData);
    TByte* testNewData0=testNewData.data();

    if (!_patch_mem_stream(testNewData0,testNewData0+testNewData.size(),
                           oldData,oldData_end,diff,diff_end))
        return false;
    for (TUInt i=0; i<(TUInt)testNewData.size(); ++i) {
        if (testNewData[i]!=newData[i])
            return false;
    }
    return true;
}

enum TDiffType{
    kDiffO,
    kDiffZ,
    kDiffZs,
    kDiffS,
    kDiffSs,
    kDiffi,
};
static const size_t kDiffTypeCount=kDiffi+1;

#ifdef _AttackPacth_ON

hpatch_BOOL _sspatch_onDiffInfo(struct sspatch_listener_t* listener,
                                const hpatch_singleCompressedDiffInfo* info,
                                hpatch_TDecompress** out_decompressPlugin,
                                unsigned char** out_temp_cache,
                                unsigned char** out_temp_cacheEnd){
    size_t temp_cache_size=info->stepMemSize+hpatch_kFileIOBufBetterSize*3;
    static TAutoMem _buf;
    if (_buf.size()<temp_cache_size)
        _buf.realloc(temp_cache_size);
    unsigned char* temp_cache=_buf.data();
    *out_temp_cache=temp_cache;
    *out_temp_cacheEnd=temp_cache+temp_cache_size;
    *out_decompressPlugin=decompressPlugin;
    return hpatch_TRUE;
}

struct TPatchiListener:public hpatchi_listener_t{
    hpatch_decompressHandle decompresser;
    hpatch_TDecompress*     decompressPlugin;
    inline TPatchiListener():decompresser(0){}
    inline ~TPatchiListener(){ if (decompresser) decompressPlugin->close(decompressPlugin,decompresser); }
    const hpi_byte* diffData_cur;
    const hpi_byte* diffData_end;
    hpatch_TStreamInput diffStream;
    hpi_pos_t           uncompressSize;
    const hpi_byte* newData_cur;
    const hpi_byte* newData_end;
    const hpi_byte* oldData;
    const hpi_byte* oldData_end;

    static hpi_BOOL _read_diff(hpi_TInputStreamHandle inputStream,hpi_byte* out_data,hpi_size_t* data_size){
        TPatchiListener& self=*(TPatchiListener*)inputStream;
        const hpi_byte* cur=self.diffData_cur;
        size_t d_size=self.diffData_end-cur;
        size_t r_size=*data_size;
        if (r_size>d_size){
            r_size=d_size;
            *data_size=(hpi_size_t)r_size;
        }
        memcpy(out_data,cur,r_size);
        self.diffData_cur=cur+r_size;
        return hpi_TRUE;
    }
    static hpi_BOOL _read_diff_dec(hpi_TInputStreamHandle inputStream,hpi_byte* out_data,hpi_size_t* data_size){
        TPatchiListener& self=*(TPatchiListener*)inputStream;
        hpi_size_t r_size=*data_size;
        if (r_size>self.uncompressSize){
            r_size=(hpi_size_t)self.uncompressSize;
            *data_size=(hpi_size_t)self.uncompressSize;
        }
        if (!self.decompressPlugin->decompress_part(self.decompresser,out_data,out_data+r_size))
            return hpi_FALSE;
        self.uncompressSize-=r_size;
        return hpi_TRUE;
    }
    static hpi_BOOL _write_new(struct hpatchi_listener_t* listener,const hpi_byte* data,hpi_size_t data_size){
        TPatchiListener& self=*(TPatchiListener*)listener;
        if (data_size>(size_t)(self.newData_end-self.newData_cur)) 
            return hpi_FALSE;
        if (0!=memcmp(self.newData_cur,data,data_size))
            return hpi_FALSE;
        self.newData_cur+=data_size;
        return hpi_TRUE;
    }
    static hpi_BOOL _read_old(struct hpatchi_listener_t* listener,hpi_pos_t read_from_pos,hpi_byte* out_data,hpi_size_t data_size){
        TPatchiListener& self=*(TPatchiListener*)listener;
        size_t dsize=self.oldData_end-self.oldData;
        if ((read_from_pos>dsize)|(data_size>(size_t)(dsize-read_from_pos))) return hpi_FALSE;
        memcpy(out_data,self.oldData+(size_t)read_from_pos,data_size);
        return hpi_TRUE;
    }
};


long attackPacth(TByte* out_newData,TByte* out_newData_end,
                        const TByte* oldData,const TByte* oldData_end,
                        const TByte* diffData,const TByte* diffData_end,
                        const char* error_tag,TDiffType diffType){
    switch (diffType){
        case kDiffO: {
            hpatch_BOOL rt0=patch(out_newData,out_newData_end,oldData,oldData_end,diffData,diffData_end);
            hpatch_BOOL rt1=_patch_mem_stream(out_newData,out_newData_end,oldData,oldData_end,diffData,diffData_end);
            if (rt0!=rt1){
                printf("\n attackPacth error!!! tag:%s\n",error_tag);
                return 1;
            }
        } break;
        case kDiffZ: {
            patch_decompress_mem(out_newData,out_newData_end,oldData,oldData_end,
                                 diffData,diffData_end,decompressPlugin);
        } break;
        case kDiffS: {
            sspatch_listener_t listener={0,_sspatch_onDiffInfo,0};
            patch_single_stream_mem(&listener,out_newData,out_newData_end,oldData,oldData_end,
                                    diffData,diffData_end,0);
        } break;
        case kDiffi: {
            hpi_compressType    compressType;
            check_lite_diff_open(diffData,diffData_end,&compressType);
            check_lite_diff(out_newData,out_newData_end,oldData,oldData_end,
                            diffData,diffData_end,decompressPlugin);
        } break;
    }
    return 0;
}

long attackPacth(TInt newSize,const TByte* oldData,const TByte* oldData_end,
                 const TByte* _diffData,const TByte* _diffData_end,int seed,TDiffType diffType){
    char tag[250]="\0";
    srand(seed);
    const long kLoopCount=1000;
    long exceptionCount=0;
    std::vector<TByte> _newData(newSize);
    TByte* newData=_newData.data();
    TByte* newData_end=newData+_newData.size();
    const TInt diffSize=_diffData_end-_diffData;
    std::vector<TByte> new_diffData(diffSize);
    TByte* diffData=new_diffData.data();
    TByte* diffData_end=diffData+diffSize;
    try {
        for (long i=0; i<kLoopCount; ++i) {
            sprintf(tag, "attackPacth exceptionCount=%ld testSeed=%d i=%ld",exceptionCount,seed,i);
            memcpy(diffData,_diffData,_diffData_end-_diffData);
            const long randCount=(long)(1+rand()*(1.0/RAND_MAX)*rand()*(1.0/RAND_MAX)*diffSize/3);
            for (long r=0; r<randCount; ++r){
                diffData[rand()%diffSize]=rand();
            }
            exceptionCount+=attackPacth(newData,newData_end,oldData,oldData_end,diffData,diffData_end,tag,diffType);
        }
        return exceptionCount;
    } catch (...) {
        printf("exception!!! tag:%s\n",tag);
        return exceptionCount+1;
    }
}
#endif

long test(const TByte* newData,const TByte* newData_end,
          const TByte* oldData,const TByte* oldData_end,const char* tag,hpatch_StreamPos_t* out_diffSizes){
    printf("%s newSize:%ld oldSize:%ld ",tag, (long)(newData_end-newData), (long)(oldData_end-oldData));
    long result=0;
    {//test diffs
        std::vector<TByte> diffData;
        create_single_compressed_diff(newData,newData_end,oldData,oldData_end,diffData,compressPlugin);
        if (out_diffSizes) out_diffSizes[kDiffS]+=diffData.size();
        if (!check_single_compressed_diff(newData,newData_end,oldData,oldData_end,
                                          diffData.data(),diffData.data()+diffData.size(),decompressPlugin)){
            printf("\n diffs error!!! tag:%s\n",tag);
            ++result;
        }else{
            printf(" diffs:%ld", (long)(diffData.size()));
#ifdef _AttackPacth_ON
            long exceptionCount=attackPacth(newData_end-newData,oldData,oldData_end,
                                            diffData.data(),diffData.data()+diffData.size(),rand(),kDiffS);
            if (exceptionCount>0) return exceptionCount;
#endif
        }
    }
    {//test diffs stream
        std::vector<TByte> diffData;
        struct hpatch_TStreamInput  newStream;
        struct hpatch_TStreamInput  oldStream;
        TVectorAsStreamOutput out_diffStream(diffData);
        mem_as_hStreamInput(&newStream,newData,newData_end);
        mem_as_hStreamInput(&oldStream,oldData,oldData_end);

        create_single_compressed_diff_stream(&newStream,&oldStream,&out_diffStream,compressPlugin,1<<4);
        if (out_diffSizes) out_diffSizes[kDiffSs]+=diffData.size();
        struct hpatch_TStreamInput in_diffStream;
        mem_as_hStreamInput(&in_diffStream,diffData.data(),diffData.data()+diffData.size());
        if (!check_single_compressed_diff(&newStream,&oldStream,&in_diffStream,decompressPlugin)){
            printf("\n diffs stream error!!! tag:%s\n",tag);
            ++result;
        }else{
            printf(" diffs stream:%ld", (long)(diffData.size()));
#ifdef _AttackPacth_ON
            long exceptionCount=attackPacth(newData_end-newData,oldData,oldData_end,
                                            diffData.data(),diffData.data()+diffData.size(),rand(),kDiffS);
            if (exceptionCount>0) return exceptionCount;
#endif
        }
    }
    {//test diffz
        std::vector<TByte> diffData;
        create_compressed_diff(newData,newData_end,oldData,oldData_end,diffData,compressPlugin);
        if (out_diffSizes) out_diffSizes[kDiffZ]+=diffData.size();
        if (!check_compressed_diff(newData,newData_end,oldData,oldData_end,
                                   diffData.data(),diffData.data()+diffData.size(),decompressPlugin)){
            printf("\n diffz error!!! tag:%s\n",tag);
            ++result;
        }else{
            printf(" diffz:%ld", (long)(diffData.size()));
#ifdef _AttackPacth_ON
            long exceptionCount=attackPacth(newData_end-newData,oldData,oldData_end,
                                            diffData.data(),diffData.data()+diffData.size(),rand(),kDiffZ);
            if (exceptionCount>0) return exceptionCount;
#endif
        }
    }
    {//test diffz stream
        std::vector<TByte> diffData;
        struct hpatch_TStreamInput  newStream;
        struct hpatch_TStreamInput  oldStream;
        TVectorAsStreamOutput out_diffStream(diffData);
        mem_as_hStreamInput(&newStream,newData,newData_end);
        mem_as_hStreamInput(&oldStream,oldData,oldData_end);

        create_compressed_diff_stream(&newStream,&oldStream,&out_diffStream,compressPlugin,1<<4);
        if (out_diffSizes) out_diffSizes[kDiffZs]+=diffData.size();
        struct hpatch_TStreamInput in_diffStream;
        mem_as_hStreamInput(&in_diffStream,diffData.data(),diffData.data()+diffData.size());
        if (!check_compressed_diff(&newStream,&oldStream,&in_diffStream,decompressPlugin)){
            printf("\n diffz stream error!!! tag:%s\n",tag);
            ++result;
        }else{
            printf(" diffz stream:%ld", (long)(diffData.size()));
#ifdef _AttackPacth_ON
            long exceptionCount=attackPacth(newData_end-newData,oldData,oldData_end,
                                            diffData.data(),diffData.data()+diffData.size(),rand(),kDiffZ);
            if (exceptionCount>0) return exceptionCount;
#endif
        }
    }
    {//test diffi
        std::vector<TByte> diffData;
        hdiffi_TCompress compressPlugini={compressPlugin,compressHpiType};
        create_lite_diff(newData,newData_end,oldData,oldData_end,diffData,&compressPlugini);
        if (out_diffSizes) out_diffSizes[kDiffi]+=diffData.size();
        if (!check_lite_diff(newData,newData_end,oldData,oldData_end,
                             diffData.data(),diffData.data()+diffData.size(),decompressPlugin)){
            printf("\n diffi error!!! tag:%s\n",tag);
            ++result;
        }else{
            printf(" diffi:%ld", (long)(diffData.size()));
#ifdef _AttackPacth_ON
            long exceptionCount=attackPacth(newData_end-newData,oldData,oldData_end,
                                            diffData.data(),diffData.data()+diffData.size(),rand(),kDiffi);
            if (exceptionCount>0) return exceptionCount;
#endif
        }
    }
    {//test diff
        std::vector<TByte> diffData;
        create_diff(newData,newData_end,oldData,oldData_end, diffData);
        if (out_diffSizes) out_diffSizes[kDiffO]+=diffData.size();
        if ((!check_diff(newData,newData_end,oldData,oldData_end,diffData.data(),diffData.data()+diffData.size()))
            ||(!check_diff_stream(newData,newData_end,oldData,oldData_end,
                                  diffData.data(),diffData.data()+diffData.size())) ){
            printf("\n diffo error!!! tag:%s\n",tag);
            ++result;
        }else{
            printf(" diffo:%ld\n", (long)(diffData.size()));
#ifdef _AttackPacth_ON
            long exceptionCount=attackPacth(newData_end-newData,oldData,oldData_end,
                                            diffData.data(),diffData.data()+diffData.size(),rand(),kDiffO);
            if (exceptionCount>0) return exceptionCount;
#endif
        }
    }
    return result;
}



static inline long test(const char* newStr,const char* oldStr,const char* error_tag){
    const TByte* newData=(const TByte*)newStr;
    const TByte* oldData=(const TByte*)oldStr;
    return test(newData,newData+strlen(newStr),oldData,oldData+strlen(oldStr),error_tag,0);
}


void setRandDataSize(int kMaxDataSize,std::vector<TByte>& oldData,std::vector<TByte>& newData,
                            double sunSizeMin,double subSizeMax){
    const TInt oldSize=(TInt)(rand()*(1.0/RAND_MAX)*rand()*(1.0/RAND_MAX)*kMaxDataSize);
    const TInt newSize=(TInt)(oldSize*(sunSizeMin+rand()*(1.0/RAND_MAX)*(subSizeMax-sunSizeMin)));
    newData.resize(newSize);
    oldData.resize(oldSize);
}

void setRandData(std::vector<TByte>& data){
    for (TInt i=0; i<(TInt)data.size(); ++i)
        data[i]=rand();
}


int main(int argc, const char * argv[]){
    clock_t time1=clock();
    long errorCount=0;
    errorCount+=testCompress("","c1");
    errorCount+=testCompress("1","c2");
    errorCount+=testCompress("12","c3");
    errorCount+=testCompress("123","c4");
    errorCount+=testCompress("123456789876543212345677654321234567765432","c5");
    
    errorCount+=test("", "", "1");
    errorCount+=test("", "1", "2");
    errorCount+=test("1", "", "3");
    errorCount+=test("1", "1", "4");
    errorCount+=test("22", "11", "5");
    errorCount+=test("22", "22", "6");
    errorCount+=test("1234567890", "1234567890", "7");
    errorCount+=test("123456789876543212345677654321234567765432", "asadsdasfefw45fg4gacasc234fervsvdfdsfef4g4gr", "8");
    errorCount+=test("123456789876543212345677654321234567765432asadsdasfefw45fg4gacasc234fervsvdfdsfef4g4gr", "asadsdasfefw45fg4gacasc234fervsvdfdsfef4g4gr", "9");
    errorCount+=test("asadsdasfefw45fg4gacasc234fervsvdfdsfef4g4gr123456789876543212345677654321234567765432", "asadsdasfefw45fg4gacasc234fervsvdfdsfef4g4gr", "10");
    errorCount+=test("a123456789876543212345677654321234567765432asadsdasfefw45fg4gacasc234fervsvdfdsfef4g4gr", "asadsdasfefw45fg4gacasc234fervsvdfdsfef4g4gr", "11");
    errorCount+=test("123456789876543212345677654321234567765432asadsdasfefw45fg4gacasc234fervsvdfdsfef4g4gr1", "asadsdasfefw45fg4gacasc234fervsvdfdsfef4g4gr", "12");
    errorCount+=test("a123456789876543212345677654321234567765432asadsdasfefw45fg4gacasc234fervsvdfdsfef4g4gr1", "asadsdasfefw45fg4gacasc234fervsvdfdsfef4g4gr", "13");

    {
        const char* _strData14="a123456789876543212345677654321234567765432asadsdasfefw45fg4gacasc234fervsvdfdsfef4g4gr1";
        const TByte* data14=(const TByte*)_strData14;
        const size_t dataSize=strlen(_strData14);
        hpatch_StreamPos_t diffSize14[kDiffTypeCount]={0};
        errorCount+=test(data14, data14+dataSize,data14, data14+dataSize, "14",diffSize14);
        for (int i=0;i<kDiffTypeCount;++i){
            if (diffSize14[i]>=dataSize){
                ++errorCount;
                printf("error!!! tag:%s\n","14 test diff size");
            }
        }
    }

    const int kMaxDataSize=1024*16;
    
    std::vector<int> seeds(kRandTestCount);
    srand(0);
    for (int i=0; i<kRandTestCount; ++i)
        seeds[i]=rand();

    hpatch_StreamPos_t sumNewSize=0;
    hpatch_StreamPos_t sumOldSize=0;
    hpatch_StreamPos_t sumDiffSizes[kDiffTypeCount]={0};
    std::vector<TByte> _newData;
    std::vector<TByte> _oldData;
    for (TInt i=0; i<kRandTestCount; ++i) {
        char tag[256];
    #if defined(_MSC_VER)&&(_MSC_VER>=1400) //VC2005
        sprintf_s(tag,256, "error==%ld testSeed=%d",errorCount,seeds[i]);
    #else
        sprintf(tag, "error==%ld testSeed=%d",errorCount,seeds[i]);
    #endif

        srand(seeds[i]);

        setRandDataSize(kMaxDataSize,_oldData,_newData,0.7,1.5);
        setRandData(_oldData);
        setRandData(_newData);
        const TInt oldSize=(TInt)_oldData.size();
        const TInt newSize=(TInt)_newData.size();
        const TInt kMaxCopyCount=(TInt)sqrt((double)oldSize);
        TByte* newData=_newData.data();
        TByte* oldData=_oldData.data();
        const TInt copyCount=0+(TInt)((1-rand()*(1.0/RAND_MAX)*rand()*(1.0/RAND_MAX))*kMaxCopyCount*4);
        const TInt kMaxCopyLength=(TInt)(1+rand()*(1.0/RAND_MAX)*kMaxCopyCount*4);
        for (TInt ci=0; ci<copyCount; ++ci) {
            const TInt length=1+(TInt)(rand()*(1.0/RAND_MAX)*kMaxCopyLength);
            if ((length>oldSize*4/5)||(length>newSize*4/5)) {
                continue;
            }
            const TInt oldPos=(oldSize-length==0)?0:(TInt)(rand()*(1.0/RAND_MAX)*(oldSize-length));
            const TInt newPos=(newSize-length==0)?0:(TInt)(rand()*(1.0/RAND_MAX)*(newSize-length));
            memcpy(&newData[0]+newPos, &oldData[0]+oldPos, length);
        }
        errorCount+=test(&newData[0],&newData[0]+newSize,&oldData[0],&oldData[0]+oldSize,tag,sumDiffSizes);
        sumNewSize+=newSize;
        sumOldSize+=oldSize;
    }

    printf("\nchecked:%ld  errorCount:%ld\n",kRandTestCount,errorCount);
    printf("newSize:100%% oldSize:%2.2f%% diffOsize:%2.2f%% diffZsize:%2.2f%%(s:%2.2f%%)"
           " diffSsize:%2.2f%%(s:%2.2f%%) diffisize:%2.2f%%\n",
            sumOldSize*100.0/sumNewSize,sumDiffSizes[kDiffO]*100.0/sumNewSize,
            sumDiffSizes[kDiffZ]*100.0/sumNewSize,sumDiffSizes[kDiffZs]*100.0/sumNewSize,
            sumDiffSizes[kDiffS]*100.0/sumNewSize,sumDiffSizes[kDiffSs]*100.0/sumNewSize,
            sumDiffSizes[kDiffi]*100.0/sumNewSize);
    clock_t time2=clock();
    printf("\nrun time:%.1f s\n",(time2-time1)*(1.0/CLOCKS_PER_SEC));

    return (int)errorCount;
}
