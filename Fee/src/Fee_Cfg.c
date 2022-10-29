#include "Fee.h"




static const Fee_ClusterType Fee_FeeClusterGroup_0[2] = 
 {
     {
         0U,
         2048U 
     }, 
     { 
         2048U, 
         2048U  
     }
 };


  const Fee_ClusterGroupType Fee_ClrGrps[FEE_NUMBER_OF_CLUSTER_GROUPS] = 
 { 
     {
         Fee_FeeClusterGroup_0, 
         2U, 
         256U 
     }
 };


 const Fee_BlockConfigType Fee_BlockConfig[FEE_CRT_CFG_NR_OF_BLOCKS] =
{
    {
        "TEST1",
        1,
        8U,
		0U,
        (boolean)TRUE,
        FEE_PROJECT_RESERVED
    },
    {
        "TEST2",
        2,
        4U,
		0U,
        (boolean)TRUE,
        FEE_PROJECT_RESERVED
    },
    {
        "TEST3",
        3,
        4U,
		0U,
        (boolean)TRUE,
        FEE_PROJECT_RESERVED
    },
};
