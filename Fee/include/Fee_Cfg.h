
 #ifndef FEE_CFG_H 
 #define FEE_CFG_H 
 


 #include "Fee_Types.h" 
 
 #define FEE_VIRTUAL_PAGE_SIZE               8U  
 
 #define FEE_NUMBER_OF_CLUSTER_GROUPS        1U  
 
 #define FEE_BLOCK_OVERHEAD                  32U 
 
 #define FEE_CLUSTER_OVERHEAD                32U 
 
 #define FEE_DATA_BUFFER_SIZE                128U 
 
 #define FEE_ERASED_VALUE                    0xffU 
 
 #define FEE_VALIDATED_VALUE                0x81U 
 
 #define FEE_INVALIDATED_VALUE              0x18U 
 
 #define FEE_CRT_CFG_NR_OF_BLOCKS              3


 #define FEE_MAX_NR_OF_BLOCKS        (FEE_CRT_CFG_NR_OF_BLOCKS) 
 
 extern const Fee_ClusterGroupType Fee_ClrGrps[ FEE_NUMBER_OF_CLUSTER_GROUPS ];
 
 extern const Fee_BlockConfigType Fee_BlockConfig[ FEE_CRT_CFG_NR_OF_BLOCKS ];
 
 #endif 