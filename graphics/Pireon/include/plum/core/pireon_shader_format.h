
#pragma pack(push,1)
typedef struct PrPslHeader {
    uint8_t  magic[4]; 
    uint16_t version_major; 
    uint16_t version_minor; 
    uint32_t bytecode_size; 
    uint32_t stage_flags;   
    uint32_t reserved;      
} PrPslHeader;
#pragma pack(pop)