// demux_3do_to_avi.cpp
// Tiny 3DO STREAM/ZSTREAM demux skeleton -> AVI (Cinepak fourcc 'cvid')
// (c) public domain / do-what-you-want. No warranty.

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iomanip>

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

static inline u32 be32(const u8* p){ return (u32(p[0])<<24)|(u32(p[1])<<16)|(u32(p[2])<<8)|u32(p[3]); }
static inline void write_le16(std::ofstream& f, u16 v){ f.put((char)(v & 0xFF)); f.put((char)((v>>8)&0xFF)); }
static inline void write_le32(std::ofstream& f, u32 v){
    f.put((char)(v & 0xFF)); f.put((char)((v>>8)&0xFF));
    f.put((char)((v>>16)&0xFF)); f.put((char)((v>>24)&0xFF));
}

// Small helpers for printing tags
static std::string tag_to_ascii(u32 tag){
    std::string s(4,'?');
    s[0] = char((tag>>24)&0xFF);
    s[1] = char((tag>>16)&0xFF);
    s[2] = char((tag>>8) &0xFF);
    s[3] = char((tag)     &0xFF);
    for(char& c: s){ if(c < 32 || c > 126) c = '.'; }
    return s;
}

struct Frame { std::vector<u8> data; };

struct Options {
    bool probe=false, extract=false;
    std::string in, out="out.avi";
    u32 video_tag=0;
    int width=0, height=0, fps=15;
};

static void usage(){
    std::cout <<
R"(demux3do --probe <infile>
demux3do --extract <infile> --video-tag 0xXXXXXXXX --width W --height H --fps N --out out.avi

Notes:
- First run --probe to discover chunk tags (you'll see ASCII like 'VID0', 'SDX2', etc.).
- Then pass the video tag (holding Cinepak frames) to --extract.
)";
}

// Parse args
static bool parse_args(int argc, char** argv, Options& o){
    for(int i=1;i<argc;i++){
        std::string a=argv[i];
        if(a=="--probe"){ o.probe=true; }
        else if(a=="--extract"){ o.extract=true; }
        else if(a=="--out" && i+1<argc){ o.out=argv[++i]; }
        else if(a=="--video-tag" && i+1<argc){
            std::string v=argv[++i];
            if(v.rfind("0x",0)==0 || v.rfind("0X",0)==0) o.video_tag = (u32)std::stoul(v,nullptr,16);
            else if(v.size()==4){ // allow ASCII like VID0
                o.video_tag = (u32(u8(v[0]))<<24)|(u32(u8(v[1]))<<16)|(u32(u8(v[2]))<<8)|(u32(u8(v[3])));
            } else { std::cerr<<"--video-tag needs 0xHEX or 4-char ASCII\n"; return false; }
        }
        else if(a=="--width" && i+1<argc){ o.width=std::stoi(argv[++i]); }
        else if(a=="--height" && i+1<argc){ o.height=std::stoi(argv[++i]); }
        else if(a=="--fps" && i+1<argc){ o.fps=std::stoi(argv[++i]); }
        else if(a[0]!='-'){ o.in=a; }
        else { std::cerr<<"Unknown arg: "<<a<<"\n"; return false; }
    }
    if(o.in.empty()){ std::cerr<<"Missing input file\n"; return false; }
    if(o.probe == o.extract){ std::cerr<<"Choose exactly one: --probe OR --extract\n"; return false; }
    if(o.extract){
        if(!o.video_tag || o.width<=0 || o.height<=0 || o.fps<=0){
            std::cerr<<"--extract requires --video-tag, --width, --height, --fps\n";
            return false;
        }
    }
    return true;
}

// PROBE: print [offset] tag size ascii
static int run_probe(const Options& o){
    std::ifstream f(o.in, std::ios::binary);
    if(!f){ std::cerr<<"Can't open "<<o.in<<"\n"; return 1; }
    u64 off=0;
    while(true){
        u8 hdr[8];
        f.read((char*)hdr,8);
        if(!f) break;
        u32 tag = be32(hdr+0);
        u32 sz  = be32(hdr+4);
        std::cout<<std::hex<<std::setw(8)<<std::setfill('0')<<off
                 <<"  tag="<<std::setw(8)<<tag
                 <<" '"<<tag_to_ascii(tag)<<"'"
                 <<"  size="<<std::setw(8)<<sz<<std::dec<<"\n";
        // Skip payload
        f.seekg(sz, std::ios::cur);
        off += 8 + sz;
    }
    return 0;
}

// AVI writing (very small, video-only, 'cvid')
static void write_avi_video_only(const Options& o, const std::vector<Frame>& frames){
    std::ofstream out(o.out, std::ios::binary);
    if(!out) throw std::runtime_error("Cannot open output AVI");

    const u32 fps = (u32)o.fps;
    const u32 microsec_per_frame = 1000000u / fps;
    const u32 width = (u32)o.width, height=(u32)o.height;

    // Precompute movi size & idx1
    u32 movi_payload = 0;
    for(const auto& fr: frames){
        u32 chunksz = (u32)fr.data.size();
        movi_payload += 8 + ((chunksz + 1) & ~1u); // '00dc'+size+data+padd
    }
    u32 idx1_size = (u32)(frames.size() * 16);

    // We'll fill RIFF sizes later; remember positions.
    auto tell = [&](std::ofstream& s){ return (u32)s.tellp(); };

    // RIFF AVI
    out.write("RIFF",4); write_le32(out, 0); // size later
    out.write("AVI ",4);

    // LIST hdrl
    out.write("LIST",4); write_le32(out, 0); // hdrl size later
    u32 hdrl_list_pos = tell(out);
    out.write("hdrl",4);

    // avih
    out.write("avih",4); write_le32(out, 56);
    write_le32(out, microsec_per_frame);
    write_le32(out, 0); // dwMaxBytesPerSec (0 ok)
    write_le32(out, 0); // dwPaddingGranularity
    write_le32(out, 0x10); // dwFlags (AVIF_HASINDEX)
    write_le32(out, (u32)frames.size()); // dwTotalFrames
    write_le32(out, 0); // dwInitialFrames
    write_le32(out, 1); // dwStreams
    write_le32(out, 0); // dwSuggestedBufferSize
    write_le32(out, width);
    write_le32(out, height);
    write_le32(out, 0); write_le32(out,0); write_le32(out,0); write_le32(out,0); // dwReserved[4]

    // LIST strl
    out.write("LIST",4); write_le32(out, 0); // strl size later
    u32 strl_list_pos = tell(out);
    out.write("strl",4);

    // strh (video)
    out.write("strh",4); write_le32(out, 56);
    out.write("vids",4);          // fccType
    out.write("cvid",4);          // fccHandler
    write_le32(out, 0);           // dwFlags
    write_le16(out, 0); write_le16(out, 0); // wPriority,wLanguage
    write_le32(out, 0);           // dwInitialFrames
    write_le32(out, 1000);        // dwScale
    write_le32(out, 1000*fps);    // dwRate
    write_le32(out, 0);           // dwStart
    write_le32(out, (u32)frames.size()); // dwLength (in dwScale units; here 1/frame)
    write_le32(out, 0);           // dwSuggestedBufferSize
    write_le32(out, 0);           // dwQuality (0 = default)
    write_le32(out, 0);           // dwSampleSize
    write_le16(out, 0); write_le16(out, 0); // rcFrame left, top
    write_le16(out, (u16)width); write_le16(out, (u16)height); // right,bottom

    // strf (BITMAPINFOHEADER)
    out.write("strf",4); write_le32(out, 40);
    write_le32(out, 40);           // biSize
    write_le32(out, width);
    write_le32(out, (u32)height);
    write_le16(out, 1);            // biPlanes
    write_le16(out, 24);           // biBitCount (ignored for cvid)
    write_le32(out, 0x64697663);   // 'cvid' LE
    write_le32(out, 0);            // biSizeImage
    write_le32(out, 0);            // biXPelsPerMeter
    write_le32(out, 0);            // biYPelsPerMeter
    write_le32(out, 0);            // biClrUsed
    write_le32(out, 0);            // biClrImportant

    // Patch LIST strl size
    u32 end_strl = tell(out);
    out.seekp(strl_list_pos-4); write_le32(out, end_strl - strl_list_pos + 4);
    out.seekp(end_strl);

    // Patch LIST hdrl size
    u32 end_hdrl = tell(out);
    out.seekp(hdrl_list_pos-4); write_le32(out, end_hdrl - hdrl_list_pos + 4);
    out.seekp(end_hdrl);

    // LIST movi
    out.write("LIST",4); write_le32(out, movi_payload + 4);
    out.write("movi",4);

    // Write frames
    struct IdxEnt { u32 ckid; u32 flags; u32 off; u32 size; };
    std::vector<IdxEnt> idx;
    idx.reserve(frames.size());
    for(const auto& fr: frames){
        // '00dc' chunk id (stream 00, compressed video)
        out.write("00dc",4);
        write_le32(out, (u32)fr.data.size());
        u32 off = tell(out); off -= 8; // offset from start of 'movi' list payload area; we'll fix base later
        out.write((const char*)fr.data.data(), fr.data.size());
        if(fr.data.size() & 1) out.put('\0'); // pad to even
        IdxEnt e; e.ckid = 0x63643030; /*'00dc' LE*/ e.flags=0x10 /*keyframe*/; e.off=off; e.size=(u32)fr.data.size();
        idx.push_back(e);
    }

    // idx1
    out.write("idx1",4); write_le32(out, idx1_size);
    // We need the absolute offset of first byte after "movi"
    // Calculate 'base' = position of first chunk data relative to start of 'movi' list data.
    // In classic AVI, idx1 offsets are from start of 'movi' list data (immediately after 'movi' FOURCC).
    u32 file_end = tell(out);
    // To compute correct offsets we need where 'movi' started:
    // RIFF(12) + LIST hdrl(...) + LIST movi(8 header + 'movi' tag)
    // We tracked nothing; simpler approach: recompute by scanning the written file length — but we *know*
    // idx entries were captured with off = currentPos-8 at write time, which equals offset from start of that chunk header.
    // For simplicity, we'll recompute base properly: reopen and scan until 'movi'.
    out.flush(); out.close();

    // Reopen to patch sizes & write idx1 offsets correctly
    std::fstream raf(o.out, std::ios::in|std::ios::out|std::ios::binary);
    if(!raf) throw std::runtime_error("Reopen failed");

    // Find 'movi'
    raf.seekg(0, std::ios::beg);
    u32 riff_size=0; raf.seekg(4); raf.read((char*)&riff_size,4); // little-endian already in file
    // Scan for 'movi'
    raf.seekg(0, std::ios::beg);
    u8 buf[4];
    u32 movi_pos = 0;
    while(raf.read((char*)buf,4)){
        if(std::memcmp(buf,"movi",4)==0){ movi_pos = (u32)raf.tellg(); break; }
    }
    if(movi_pos==0) throw std::runtime_error("Couldn't locate 'movi'");

    // idx1 starts at end of file minus idx1 size+8
    raf.seekg(0, std::ios::end);
    u32 fsize = (u32)raf.tellg();
    u32 idx1_start = fsize - (idx1_size + 8);
    raf.seekp(idx1_start + 8); // after 'idx1'+size

    // Now write idx1 with proper offsets from start of movi data (i.e., movi_pos)
    // Re-scan movi to find each chunk file offset; but we stored off as "pos-8 relative to file start", so:
    // We will adjust: idx_off = (chunk_file_pos) - (movi_pos)
    // To get chunk_file_pos we need to have captured it; we didn't store absolute file positions. Quick fix:
    // Rederive by rescanning chunks while writing idx1 (simple and robust).

    // Rescan chunks to write idx1
    raf.seekg(movi_pos, std::ios::beg);
    // movi data begins *after* 'movi' tag, so the first chunk starts here.
    std::vector<IdxEnt> idx_fix; idx_fix.reserve(idx.size());
    while(idx_fix.size()<idx.size()){
        u32 ckid_le, cksz_le;
        if(!raf.read((char*)&ckid_le,4)) break;
        if(!raf.read((char*)&cksz_le,4)) break;
        u32 chunk_pos = (u32)raf.tellg() - 8;
        // Store if it's '00dc'
        if(ckid_le == 0x63643030){ // '00dc'
            IdxEnt e; e.ckid=ckid_le; e.flags=0x10; e.off = chunk_pos - (movi_pos); e.size = cksz_le;
            idx_fix.push_back(e);
        }
        // skip payload (+ pad)
        raf.seekg(cksz_le + (cksz_le & 1), std::ios::cur);
    }
    if(idx_fix.size()!=idx.size()) std::cerr<<"[warn] idx sizes differ (still playable; many players ignore idx1)\n";

    // Write idx1 entries
    for(const auto& e: (idx_fix.empty()?idx:idx_fix)){
        raf.write((const char*)&e.ckid,4);
        write_le32((std::ofstream&)raf, e.flags);
        write_le32((std::ofstream&)raf, e.off);
        write_le32((std::ofstream&)raf, e.size);
    }

    // Patch RIFF size
    raf.seekp(4, std::ios::beg);
    u32 riff_final = fsize - 8; // RIFF size excludes 'RIFF'+size
    write_le32((std::ofstream&)raf, riff_final);
    raf.close();
}

// Extract frames for a single video tag
static int run_extract(const Options& o){
    std::ifstream f(o.in, std::ios::binary);
    if(!f){ std::cerr<<"Can't open "<<o.in<<"\n"; return 1; }

    std::vector<Frame> frames;
    u64 off=0; size_t n_video=0;
    while(true){
        u8 hdr[8];
        f.read((char*)hdr,8);
        if(!f) break;
        u32 tag = be32(hdr+0);
        u32 sz  = be32(hdr+4);
        if(tag == o.video_tag){
            Frame fr; fr.data.resize(sz);
            f.read((char*)fr.data.data(), sz);
            if(!f){ std::cerr<<"Truncated at video payload\n"; return 1; }
            frames.push_back(std::move(fr));
            n_video++;
        }else{
            f.seekg(sz, std::ios::cur);
        }
        off += 8 + sz;
    }
    if(frames.empty()){
        std::cerr<<"No frames found for tag 0x"<<std::hex<<o.video_tag<<" ('"<<tag_to_ascii(o.video_tag)<<"')\n";
        return 1;
    }

    std::cout<<"Collected "<<frames.size()<<" video frames for tag 0x"<<std::hex<<o.video_tag
             <<" ('"<<tag_to_ascii(o.video_tag)<<"')\n";

    try{
        write_avi_video_only(o, frames);
        std::cout<<"Wrote "<<o.out<<" (AVI/cvid)\n";
        std::cout<<"Now you can do: ffplay \""<<o.out<<"\"  or  ffmpeg -i \""<<o.out<<"\" -c:v libx264 -crf 18 out.mp4\n";
    }catch(const std::exception& e){
        std::cerr<<"AVI write failed: "<<e.what()<<"\n";
        return 1;
    }
    return 0;
}

int main(int argc, char** argv){
    Options o;
    if(!parse_args(argc, argv, o)){ usage(); return 1; }
    try{
        if(o.probe) return run_probe(o);
        if(o.extract) return run_extract(o);
    }catch(const std::exception& e){
        std::cerr<<"Error: "<<e.what()<<"\n";
        return 1;
    }
    return 0;
}
