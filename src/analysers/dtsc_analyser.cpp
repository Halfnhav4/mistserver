/// \file dtsc_analyser.cpp
/// Reads an DTSC file and prints all readable data about it

#include <string>
#include <mist/dtsc.h>
#include <mist/json.h>
#include <mist/config.h>

/// Reads an DTSC file and prints all readable data about it
int main(int argc, char ** argv){
  Util::Config conf = Util::Config(argv[0], PACKAGE_VERSION);
  conf.addOption("filename", JSON::fromString("{\"arg_num\":1, \"arg\":\"string\", \"help\":\"Filename of the DTSC file to analyse.\"}"));
  conf.parseArgs(argc, argv);
  DTSC::File F(conf.getString("filename"));
  std::string loader = F.getHeader();
  JSON::Value meta = JSON::fromDTMI(loader);
  std::cout << meta.toPrettyString() << std::endl;
  JSON::Value pack;

  long long unsigned int firstpack = 0;
  long long unsigned int nowpack = 0;
  long long unsigned int lastaudio = 0;
  long long unsigned int lastvideo = 0;
  long long unsigned int lastkey = 0;
  long long unsigned int totalvideo = 0;
  long long unsigned int totalaudio = 0;
  long long unsigned int keyframes = 0;
  long long unsigned int key_min = 0xffffffff;
  long long unsigned int key_max = 0;
  long long unsigned int vid_min = 0xffffffff;
  long long unsigned int vid_max = 0;
  long long unsigned int aud_min = 0xffffffff;
  long long unsigned int aud_max = 0;
  long long unsigned int bfrm_min = 0xffffffff;
  long long unsigned int bfrm_max = 0;
  long long unsigned int bps = 0;

  while (loader.size() != 0){
    loader = F.getPacket();
    if (loader.size() != 0){
      pack = JSON::fromDTMI(loader);
      std::cout << pack.toPrettyString() << std::endl;
      nowpack = pack["time"].asInt();
      if (firstpack == 0){firstpack = nowpack;}
      if (pack["datatype"].asString() == "audio"){
        if (lastaudio != 0 && (nowpack - lastaudio) != 0){
          bps = pack["data"].asString().size() / (nowpack - lastaudio);
          if (bps < aud_min){aud_min = bps;}
          if (bps > aud_max){aud_max = bps;}
        }
        totalaudio += pack["data"].asString().size();
        lastaudio = nowpack;
      }
      if (pack["datatype"].asString() == "video"){
        if (lastvideo != 0 && (nowpack - lastvideo) != 0){
          bps = pack["data"].asString().size() / (nowpack - lastvideo);
          if (bps < vid_min){vid_min = bps;}
          if (bps > vid_max){vid_max = bps;}
        }
        if (pack["keyframe"].asInt() != 0){
          if (lastkey != 0){
            bps = nowpack - lastkey;
            if (bps < key_min){key_min = bps;}
            if (bps > key_max){key_max = bps;}
          }
          keyframes++;
          lastkey = nowpack;
        }
        if (pack["offset"].asInt() != 0){
          bps = pack["offset"].asInt();
          if (bps < bfrm_min){bfrm_min = bps;}
          if (bps > bfrm_max){bfrm_max = bps;}
        }
        totalvideo += pack["data"].asString().size();
        lastvideo = nowpack;
      }
    }
  }

  std::cout << std::endl << "Summary:" << std::endl;
  meta["length"] = (long long int)((nowpack - firstpack)/1000);
  if (meta.isMember("audio")){
    meta["audio"]["bps"] = (long long int)(totalaudio / ((lastaudio - firstpack) / 1000));
    std::cout << "  Audio: " << meta["audio"]["codec"].asString() << std::endl;
    std::cout << "    Bitrate: " << meta["audio"]["bps"].asInt() << std::endl;
  }
  if (meta.isMember("video")){
    meta["video"]["bps"] = (long long int)(totalvideo / ((lastvideo - firstpack) / 1000));
    meta["video"]["keyms"] = (long long int)((lastvideo - firstpack) / keyframes);
    if (meta["video"]["keyms"].asInt() - key_min > key_max - meta["video"]["keyms"].asInt()){
      meta["video"]["keyvar"] = (long long int)(meta["video"]["keyms"].asInt() - key_min);
    }else{
      meta["video"]["keyvar"] = (long long int)(key_max - meta["video"]["keyms"].asInt());
    }
    std::cout << "  Video: " << meta["video"]["codec"].asString() << std::endl;
    std::cout << "    Bitrate: " << meta["video"]["bps"].asInt() << std::endl;
    std::cout << "    Keyframes: " << meta["video"]["keyms"].asInt() << "~" << meta["video"]["keyvar"].asInt() << std::endl;
    std::cout << "    B-frames: " << bfrm_min << " - " << bfrm_max << std::endl;
  }
  return 0;
}//main
