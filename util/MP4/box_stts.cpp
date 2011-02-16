#include "box.cpp"
#include <string>
#include <vector>

struct stts_record {
  uint32_t SampleCount;
  uint32_t SampleDelta;
};//stsc_record

class Box_stts {
  public:
    Box_stts( );
    ~Box_stts();
    Box * GetBox();
    void SetReserved( );
    void AddEntry( uint32_t SampleCount, uint32_t SampleDelta, uint32_t Offset = 0 );
    void WriteContent( );
  private:
    Box * Container;
    
    std::vector<stts_record> Entries;
};//Box_ftyp Class

Box_stts::Box_stts( ) {
  Container = new Box( 0x73747473 );
  SetReserved();
}

Box_stts::~Box_stts() {
  delete Container;
}

Box * Box_stts::GetBox() {
  return Container;
}

void Box_stts::SetReserved( ) {
  Container->SetPayload((uint32_t)4,Box::uint32_to_uint8(0));
}

void Box_stts::AddEntry( uint32_t SampleCount, uint32_t SampleDelta, uint32_t Offset ) {
  if(Offset >= Entries.size()) {
    Entries.resize(Offset+1);
  }
  Entries[Offset].SampleCount = SampleCount;
  Entries[Offset].SampleDelta = SampleDelta;
}


void Box_stts::WriteContent( ) {
  Container->ResetPayload();
  SetReserved( );
  if(!Entries.empty()) {
    for(int32_t i = Entries.size() -1; i >= 0; i--) {
      Container->SetPayload((uint32_t)4,Box::uint32_to_uint8(Entries[i].SampleDelta),(i*8)+12);
      Container->SetPayload((uint32_t)4,Box::uint32_to_uint8(Entries[i].SampleCount),(i*8)+8);
    }
  }
  Container->SetPayload((uint32_t)4,Box::uint32_to_uint8(Entries.size()),4);
}