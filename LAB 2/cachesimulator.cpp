/*
   Cache Simulator
   Level one L1 and level two L2 cache parameters are read from file (block size, line per set and set per cache).
   The 32 bit address is divided into tag bits (t), set index bits (s) and block offset bits (b)
   s = log2(#sets)   b = log2(block size)  t=32-s-b
*/
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <iomanip>
#include <stdlib.h>
#include <cmath>
#include <bitset>

using namespace std;
//access state:
#define NA 0 // no action
#define RH 1 // read hit
#define RM 2 // read miss
#define WH 3 // Write hit
#define WM 4 // write miss

#define ADDRESS_LENGTH 32

struct config{
    int L1blocksize;
    int L1setsize;
    int L1size;
    int L2blocksize;
    int L2setsize;
    int L2size;
};

/* you can define the cache class here, or design your own data structure for L1 and L2 cache
*/       
class cache {
    struct node {
        unsigned long tag;
        unsigned long counter;
        short valid;
        node(): tag(0), counter(0), valid(-1) {}
    };


public:
    cache(int blockSize, int setSize, int cacheSize) {
        if (setSize == 0) {
            setSize = cacheSize * 1024 / blockSize;
        }
        indexSize_ = (cacheSize * 1024) / (blockSize * setSize);
        indexBitSize_ = log2(indexSize_);
        setSize_ = setSize;
        blockOffset_ = log2(blockSize);
        tagBitSize_ = ADDRESS_LENGTH - indexBitSize_ - blockOffset_;

        cache_.resize(indexSize_);
        for (unsigned long i = 0; i < indexSize_; ++i) {
            cache_[i].resize(setSize_);
        }
        // std::cout <<  indexSize_ << std::endl;
        // std::cout << setSize_ << std::endl;
    }

    unsigned long getTag(const bitset<32> addr) const {
        unsigned long tag = 0;
        // std::cout << tagBitSize_ << std::endl;
        const std::string& addr_str = addr.to_string();
        for (size_t i = 0; i < tagBitSize_; ++i) {
            if (addr_str[i] == '1') {
                tag += (1 << (tagBitSize_ - i - 1));
                // std::cout << "1";
            } else {
                // std::cout << "0";
            }
        }
        // std::cout << std::endl;
        return tag;
    }

    unsigned long getIndex(const bitset<32> addr) const {
        unsigned long index = 0;
        const std::string& addr_str = addr.to_string();
        for (size_t i = 0; i < indexBitSize_; i++) {
            if (addr_str[tagBitSize_ + i] == '1') {
                index += (1 << (indexBitSize_ - i - 1));
            } else {
                // std::cout << "0";
            }
        }
        // std::cout << std::endl;
        return index;
    }

    unsigned long getOffset(const bitset<32> addr) const {
        unsigned long offset = 0;
        const std::string& addr_str = addr.to_string();
        for (size_t i = 0; i < blockOffset_; ++i) {
            if (addr_str[i + tagBitSize_ + indexBitSize_] == '1') {
                offset += (1 << (blockOffset_ - i - 1));
            }
        }
        return offset;
    }

    bool hiting(unsigned long index, unsigned long tag) {
        for (size_t i = 0; i < cache_[index].size(); ++i) {
            if (cache_[index][i].valid != -1 && cache_[index][i].tag == tag) {
                return true;
            }
        }
        return false;
    }

    void updateCache(unsigned long index, unsigned long tag) {
        unsigned long update_index = 0;
        for (size_t i = 0; i < cache_[index].size(); ++i) {
            if (cache_[index][i].valid != -1 && cache_[index][i].tag == tag) {
                update_index = i;
                break;
            }
            if (cache_[index][i].valid == -1) {
                update_index = i;
                break;
            }
            if (cache_[index][i].counter < cache_[index][update_index].counter) {
                update_index = i;
            }
        }
        cache_[index][update_index].counter = gen_counter;
        gen_counter += 1;
        cache_[index][update_index].valid = 1;
        cache_[index][update_index].tag = tag;
    }
private:
    unsigned long indexSize_, indexBitSize_;
    unsigned long setSize_;
    unsigned long blockOffset_;
    unsigned long tagBitSize_;

    vector<vector<node> > cache_;
    static unsigned long gen_counter;
};

unsigned long cache::gen_counter = 1;


int main(int argc, char* argv[]) {

    config cacheconfig;
    ifstream cache_params;
    string dummyLine;
    cache_params.open(argv[1]);
    while(!cache_params.eof())  // read config file
    {
        cache_params>>dummyLine;
        cache_params>>cacheconfig.L1blocksize;
        cache_params>>cacheconfig.L1setsize;              
        cache_params>>cacheconfig.L1size;
        cache_params>>dummyLine;              
        cache_params>>cacheconfig.L2blocksize;           
        cache_params>>cacheconfig.L2setsize;        
        cache_params>>cacheconfig.L2size;
    }

    // Implement by you: 
    // initialize the hirearch cache system with those configs
    // probably you may define a Cache class for L1 and L2, or any data structure you like
    
    cache cacheL1(cacheconfig.L1blocksize, cacheconfig.L1setsize, cacheconfig.L1size);
    cache cacheL2(cacheconfig.L2blocksize, cacheconfig.L2setsize, cacheconfig.L2size);

    int L1AcceState =0; // L1 access state variable, can be one of NA, RH, RM, WH, WM;
    int L2AcceState =0; // L2 access state variable, can be one of NA, RH, RM, WH, WM;

    ifstream traces;
    ofstream tracesout;
    string outname;
    outname = string(argv[2]) + ".out";

    traces.open(argv[2]);
    tracesout.open(outname.c_str());

    string line;
    string accesstype;  // the Read/Write access type from the memory trace;
    string xaddr;       // the address from the memory trace store in hex;
    unsigned int addr;  // the address from the memory trace store in unsigned int;        
    bitset<32> accessaddr; // the address from the memory trace store in the bitset;

    if (traces.is_open()&&tracesout.is_open()){    
        while (getline (traces,line)){   // read mem access file and access Cache

            istringstream iss(line); 
            if (!(iss >> accesstype >> xaddr)) {break;}
            stringstream saddr(xaddr);
            saddr >> std::hex >> addr;
            accessaddr = bitset<32> (addr);

            // std::cout << accessaddr.to_string() << std::endl;
            unsigned long l1_tag = cacheL1.getTag(accessaddr);
            unsigned long l2_tag = cacheL2.getTag(accessaddr);
            unsigned long l1_index = cacheL1.getIndex(accessaddr);
            unsigned long l2_index = cacheL2.getIndex(accessaddr);
            // std::cout << l1_index << ":" << l1_tag << std::endl;
            // std::cout << l2_index << ":" << l2_tag << std::endl;

            // access the L1 and L2 Cache according to the trace;
            if (accesstype.compare("R")==0)

            {    
                //Implement by you:
                // read access to the L1 Cache, 
                //  and then L2 (if required), 
                //  update the L1 and L2 access state variable;
                
                if (cacheL1.hiting(l1_index, l1_tag)) {
                    L1AcceState = RH;
                    L2AcceState = NA;
                    cacheL1.updateCache(l1_index, l1_tag);
                } else if (cacheL2.hiting(l2_index, l2_tag)) {
                    L1AcceState = RM;
                    L2AcceState = RH;
                    cacheL1.updateCache(l1_index, l1_tag);
                    cacheL2.updateCache(l2_index, l2_tag);
                } else {
                    L1AcceState = RM;
                    L2AcceState = RM;
                    cacheL1.updateCache(l1_index, l1_tag);
                    cacheL2.updateCache(l2_index, l2_tag);
                }
            }
            else 
            {    
                //Implement by you:
                // write access to the L1 Cache, 
                //and then L2 (if required), 
                //update the L1 and L2 access state variable;
                if (cacheL1.hiting(l1_index, l1_tag)) {
                    L1AcceState = WH;
                    cacheL1.updateCache(l1_index, l1_tag);
                    if (cacheL2.hiting(l2_index, l2_tag)) {
                      L2AcceState = WH;
                        cacheL2.updateCache(l2_index, l2_tag);
                    } else {
                      L2AcceState = NA;
                    }
                } else if (cacheL2.hiting(l2_index, l2_tag)) {
                    L1AcceState = WM;
                    L2AcceState = WH;
                    cacheL2.updateCache(l2_index, l2_tag);
                    // cacheL1.updateCache(l1_index, l1_tag);
                } else {
                    L1AcceState = WM;
                    L2AcceState = WM;
                    // cacheL1.updateCache(l1_index, l1_tag);
                    // cacheL2.updateCache(l2_index, l2_tag);
                }
            }

            tracesout<< L1AcceState << " " << L2AcceState << endl;  // Output hit/miss results for L1 and L2 to the output file;

        }
        traces.close();
        tracesout.close(); 
    }
    else cout<< "Unable to open trace or traceout file ";







    return 0;
}
