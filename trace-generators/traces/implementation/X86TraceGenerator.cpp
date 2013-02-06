/*!
 * \author Nagesh B. Lakshminarayana and Jaekyu Lee
 * \date February 4, 2011
 * \brief ptx trace generator for MacSim simulator
 */

#include <iostream>
#include <iomanip>
#include <fstream>
#include <zlib.h>
#include <string.h>
#include <errno.h>
#include <map>
#include <unistd.h>
#include <cstdlib>
#include <fstream>
#include <cfloat>
#include <iostream>


#include <traces/interface/X86TraceGenerator.h>
#include <traces/interface/X86TraceGeneratorCommon.h>

#include <ocelot/api/interface/OcelotConfiguration.h>
#include <ocelot/api/interface/ocelot.h>

#include <ocelot/executive/interface/ExecutableKernel.h>
#include <ocelot/executive/interface/EmulatedKernel.h>
#include <ocelot/executive/interface/Device.h>

#include <ocelot/ir/interface/Module.h>

#include <hydrazine/interface/Exception.h>
#include <hydrazine/interface/debug.h>

#include <boost/filesystem.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/dynamic_bitset.hpp> 


#ifdef REPORT_BASE
#undef REPORT_BASE
#endif

#define REPORT_BASE 0


#define GPU_REGISTER 49
#define GPU_SHARED 60


#define ST_INST(opcode) \
   (opcode == ir::PTXInstruction::St || \
    opcode == ir::PTXInstruction::Atom || \
    opcode == ir::PTXInstruction::Red)

#define SUR_INST(opcode) \
  (opcode == ir::PTXInstruction::Suld || \
   opcode == ir::PTXInstruction::Sust || \
   opcode == ir::PTXInstruction::Sured || \
   opcode == ir::PTXInstruction::Suq)

#define TEX_INST(opcode) \
  (opcode == ir::PTXInstruction::Tex || \
   opcode == ir::PTXInstruction::Tld4 || \
   opcode == ir::PTXInstruction::Txq)

#define BRANCH_INST(opcode) \
  (opcode == ir::PTXInstruction::Bra || \
   opcode == ir::PTXInstruction::Call || \
   opcode == ir::PTXInstruction::Ret || \
   opcode == ir::PTXInstruction::Exit)


using namespace std;


/////////////////////////////////////////////////////////////////////////////////////////////// 
// Opcode enumerator
// These should be exactly matched with the Macsim
/////////////////////////////////////////////////////////////////////////////////////////////// 
// numbering is not correct!!
/*
   //before matching with enum in trace_read.h in MacSim
enum TR_OPCODE_ENUM_  {
  XED_CATEGORY_INVALID   = 0,
  XED_CATEGORY_3DNOW,
  XED_CATEGORY_AES,
  XED_CATEGORY_AVX,
  XED_CATEGORY_BASE,
  XED_CATEGORY_BINARY,
  XED_CATEGORY_BITBYTE,
  XED_CATEGORY_BROADCAST,
  XED_CATEGORY_CALL,
  XED_CATEGORY_CMOV,
  XED_CATEGORY_COND_BR   = 10, 
  XED_CATEGORY_CONVERT,
  XED_CATEGORY_DATAXFER,
  XED_CATEGORY_DECIMAL,
  XED_CATEGORY_FCMOV,
  XED_CATEGORY_FLAGOP,
  XED_CATEGORY_INTERRUPT,
  XED_CATEGORY_IO,
  XED_CATEGORY_IOSTRINGOP,
  XED_CATEGORY_LOGICAL,
  XED_CATEGORY_MISC     = 20,
  XED_CATEGORY_MMX,
  XED_CATEGORY_NOP,
  XED_CATEGORY_PCLMULQDQ,
  XED_CATEGORY_POP,
  XED_CATEGORY_PREFETCH,
  XED_CATEGORY_PUSH,
  XED_CATEGORY_RET,
  XED_CATEGORY_ROTATE,
  XED_CATEGORY_SEGOP,
  XED_CATEGORY_SEMAPHORE = 30,
  XED_CATEGORY_SHIFT,
  XED_CATEGORY_SSE,
  XED_CATEGORY_SSE5,
  XED_CATEGORY_STRINGOP,
  XED_CATEGORY_STTNI,
  XED_CATEGORY_SYSCALL,
  XED_CATEGORY_SYSRET,
  XED_CATEGORY_SYSTEM,
  XED_CATEGORY_UNCOND_BR,
  XED_CATEGORY_VTX      = 40,
  XED_CATEGORY_WIDENOP,
  XED_CATEGORY_X87_ALU,
  XED_CATEGORY_XSAVE,
  XED_CATEGORY_XSAVEOPT,
  TR_MUL,
  TR_DIV,
  TR_FMUL,
  TR_FDIV,
  TR_NOP,
  PREFETCH_NTA         = 50,
  PREFETCH_T0,
  PREFETCH_T1,
  PREFETCH_T2,
  TR_MEM_LD_LM,
  TR_MEM_LD_SM,
  TR_MEM_LD_GM,
  TR_MEM_ST_LM,
  TR_MEM_ST_SM,
  TR_MEM_ST_GM,
  TR_DATA_XFER_LM      = 60,
  TR_DATA_XFER_SM,
  TR_DATA_XFER_GM,
  TR_MEM_LD_CM,
  TR_MEM_LD_TM,
  TR_MEM_LD_PM,
  LD_CM_CA,
  LD_CM_CG,
  LD_CM_CS,
  LD_CM_LU,
  LD_CM_CU            = 70,
  LD_GM_CA,
  LD_GM_CG,
  LD_GM_CS,
  LD_GM_LU,
  LD_GM_CU,
  LD_LM_CA,
  LD_LM_CG,
  LD_LM_CS,
  LD_LM_LU,
  LD_LM_CU            = 80,
  LD_PM_CA,
  LD_PM_CG,
  LD_PM_CS,
  LD_PM_LU,
  LD_PM_CU,
  LD_SM_CA,
  LD_SM_CG,
  LD_SM_CS,
  LD_SM_LU,
  LD_SM_CU           = 90,
  LDU_GM,
  ST_GM_WB,
  ST_GM_CG,
  ST_GM_CS,
  ST_GM_WT,
  ST_LM_WB,
  ST_LM_CG,
  ST_LM_CS,
  ST_LM_WT,
  ST_SM_WB          = 100,
  ST_SM_CG,
  ST_SM_CS,
  ST_SM_WT,
  PREF_GM_L1,
  PREF_GM_L2,
  PREF_LM_L1,
  PREF_LM_L2,
  PREF_UNIFORM,
  GPU_ABS,
  GPU_ABS64        = 110,
  GPU_ADD, 
  GPU_ADD64, 
	GPU_ADDC,
	GPU_AND,
	GPU_AND64,
	GPU_ATOM,
	GPU_ATOM64,
	GPU_BAR,
	GPU_BFE,
	GPU_BFE64        = 120,
	GPU_BFI,
	GPU_BFI64,
	GPU_BFIND,
	GPU_BFIND64,
	GPU_BRA,
	GPU_BREV,
	GPU_BREV64,
	GPU_BRKPT,
	GPU_CALL,
	GPU_CLZ         = 130,
	GPU_CLZ64,
	GPU_CNOT,
	GPU_CNOT64,
	GPU_COPYSIGN,
	GPU_COPYSIGN64,
	GPU_COS,
	GPU_CVT,
	GPU_CVT64,
	GPU_CVTA,
	GPU_CVTA64      = 140,
	GPU_DIV,
	GPU_DIV64,
	GPU_EX2,
	GPU_EXIT,
	GPU_FMA,
	GPU_FMA64,
	GPU_ISSPACEP,
	GPU_LD,
	GPU_LD64,
	GPU_LDU         = 150,
	GPU_LDU64,
	GPU_LG2,
	GPU_MAD24,
	GPU_MAD,
	GPU_MAD64,
	GPU_MAX,
	GPU_MAX64,
	GPU_MEMBAR,
	GPU_MIN,
	GPU_MIN64      = 160,
	GPU_MOV,
	GPU_MOV64,
	GPU_MUL24,
	GPU_MUL,
	GPU_MUL64,
	GPU_NEG,
	GPU_NEG64,
	GPU_NOT,
	GPU_NOT64,
	GPU_OR        = 170,
	GPU_OR64,
	GPU_PMEVENT,
	GPU_POPC,
	GPU_POPC64,
	GPU_PREFETCH,
	GPU_PREFETCHU,
	GPU_PRMT,
	GPU_RCP,
	GPU_RCP64,
	GPU_RED       = 180,
	GPU_RED64,
	GPU_REM,
	GPU_REM64,
	GPU_RET,
	GPU_RSQRT,
	GPU_RSQRT64,
	GPU_SAD,
	GPU_SAD64,
	GPU_SELP,
	GPU_SELP64    = 190,
	GPU_SET,
	GPU_SET64,
	GPU_SETP,
	GPU_SETP64,
	GPU_SHL,
	GPU_SHL64,
	GPU_SHR,
	GPU_SHR64,
	GPU_SIN,
	GPU_SLCT     = 200,
	GPU_SLCT64,
	GPU_SQRT,
	GPU_SQRT64,
	GPU_ST,
	GPU_ST64,
	GPU_SUB,
	GPU_SUB64,
	GPU_SUBC,
	GPU_SULD,
	GPU_SULD64  = 210,
	GPU_SURED,
	GPU_SURED64,
	GPU_SUST,
	GPU_SUST64,
	GPU_SUQ,
  GPU_TESTP,
  GPU_TESTP64,
  GPU_TEX,
  GPU_TLD4,
  GPU_TXQ     = 220,
  GPU_TRAP,
  GPU_VABSDIFF,
  GPU_VADD,
  GPU_VMAD,
  GPU_VMAX,
  GPU_VMIN,
  GPU_VSET,
  GPU_VSHL,
  GPU_VSHR,
  GPU_VSUB   = 230,
  GPU_VOTE,
  GPU_XOR,
  GPU_XOR64,
  GPU_RECONVERGE,
  GPU_PHI,
  TR_OPCODE_LAST
} TR_OPCODE_ENUM;
*/

enum TR_OPCODE_ENUM_  {
  XED_CATEGORY_INVALID   = 0,
  XED_CATEGORY_3DNOW,
  XED_CATEGORY_AES,
  XED_CATEGORY_AVX,
  XED_CATEGORY_BINARY,
  XED_CATEGORY_BITBYTE,
  XED_CATEGORY_BROADCAST,
  XED_CATEGORY_CALL,
  XED_CATEGORY_CMOV,
  XED_CATEGORY_COND_BR,
  XED_CATEGORY_CONVERT,
  XED_CATEGORY_DATAXFER,
  XED_CATEGORY_DECIMAL,
  XED_CATEGORY_FCMOV,
  XED_CATEGORY_FLAGOP,
  XED_CATEGORY_INTERRUPT,
  XED_CATEGORY_IO,
  XED_CATEGORY_IOSTRINGOP,
  XED_CATEGORY_LOGICAL,
  XED_CATEGORY_MISC,
  XED_CATEGORY_MMX,
  XED_CATEGORY_NOP,
  XED_CATEGORY_PCLMULQDQ,
  XED_CATEGORY_POP,
  XED_CATEGORY_PREFETCH,
  XED_CATEGORY_PUSH,
  XED_CATEGORY_RET,
  XED_CATEGORY_ROTATE,
  XED_CATEGORY_SEGOP,
  XED_CATEGORY_SEMAPHORE,
  XED_CATEGORY_SHIFT,
  XED_CATEGORY_SSE,
  XED_CATEGORY_STRINGOP,
  XED_CATEGORY_STTNI,
  XED_CATEGORY_SYSCALL,
  XED_CATEGORY_SYSRET,
  XED_CATEGORY_SYSTEM,
  XED_CATEGORY_UNCOND_BR,
  XED_CATEGORY_VTX,
  XED_CATEGORY_WIDENOP,
  XED_CATEGORY_X87_ALU,
  XED_CATEGORY_XSAVE,
  XED_CATEGORY_XSAVEOPT,
  TR_MUL,
  TR_DIV,
  TR_FMUL,
  TR_FDIV,
  TR_NOP,
  PREFETCH_NTA,
  PREFETCH_T0,
  PREFETCH_T1,
  PREFETCH_T2,
  TR_MEM_LD_LM,
  TR_MEM_LD_SM,
  TR_MEM_LD_GM,
  TR_MEM_ST_LM,
  TR_MEM_ST_SM,
  TR_MEM_ST_GM,
  TR_DATA_XFER_LM,
  TR_DATA_XFER_SM,
  TR_DATA_XFER_GM,
  TR_MEM_LD_CM,
  TR_MEM_LD_TM,
  TR_MEM_LD_PM,
  GPU_EN,
  LD_CM_CA,
  LD_CM_CG,
  LD_CM_CS,
  LD_CM_LU,
  LD_CM_CU,
  LD_GM_CA,
  LD_GM_CG,
  LD_GM_CS,
  LD_GM_LU,
  LD_GM_CU,
  LD_LM_CA,
  LD_LM_CG,
  LD_LM_CS,
  LD_LM_LU,
  LD_LM_CU,
  LD_PM_CA,
  LD_PM_CG,
  LD_PM_CS,
  LD_PM_LU,
  LD_PM_CU,
  LD_SM_CA,
  LD_SM_CG,
  LD_SM_CS,
  LD_SM_LU,
  LD_SM_CU,
  LDU_GM,
  ST_GM_WB,
  ST_GM_CG,
  ST_GM_CS,
  ST_GM_WT,
  ST_LM_WB,
  ST_LM_CG,
  ST_LM_CS,
  ST_LM_WT,
  ST_SM_WB,
  ST_SM_CG,
  ST_SM_CS,
  ST_SM_WT,
  PREF_GM_L1,
  PREF_GM_L2,
  PREF_LM_L1,
  PREF_LM_L2,
  PREF_UNIFORM,
  GPU_ABS,
  GPU_ABS64,
  GPU_ADD, 
  GPU_ADD64, 
	GPU_ADDC,
	GPU_AND,
	GPU_AND64,
	GPU_ATOM,
	GPU_ATOM64,
	GPU_BAR,
	GPU_BFE,
	GPU_BFE64,
	GPU_BFI,
	GPU_BFI64,
	GPU_BFIND,
	GPU_BFIND64,
	GPU_BRA,
	GPU_BREV,
	GPU_BREV64,
	GPU_BRKPT,
	GPU_CALL,
	GPU_CLZ,
	GPU_CLZ64,
	GPU_CNOT,
	GPU_CNOT64,
	GPU_COPYSIGN,
	GPU_COPYSIGN64,
	GPU_COS,
	GPU_CVT,
	GPU_CVT64,
	GPU_CVTA,
	GPU_CVTA64,
	GPU_DIV,
	GPU_DIV64,
	GPU_EX2,
	GPU_EXIT,
	GPU_FMA,
	GPU_FMA64,
	GPU_ISSPACEP,
	GPU_LD,
	GPU_LD64,
	GPU_LDU,
	GPU_LDU64,
	GPU_LG2,
	GPU_MAD24,
	GPU_MAD,
	GPU_MAD64,
	GPU_MAX,
	GPU_MAX64,
	GPU_MEMBAR,
	GPU_MIN,
	GPU_MIN64,
	GPU_MOV,
	GPU_MOV64,
	GPU_MUL24,
	GPU_MUL,
	GPU_MUL64,
	GPU_NEG,
	GPU_NEG64,
	GPU_NOT,
	GPU_NOT64,
	GPU_OR,
	GPU_OR64,
	GPU_PMEVENT,
	GPU_POPC,
	GPU_POPC64,
	GPU_PREFETCH,
	GPU_PREFETCHU,
	GPU_PRMT,
	GPU_RCP,
	GPU_RCP64,
	GPU_RED,
	GPU_RED64,
	GPU_REM,
	GPU_REM64,
	GPU_RET,
	GPU_RSQRT,
	GPU_RSQRT64,
	GPU_SAD,
	GPU_SAD64,
	GPU_SELP,
	GPU_SELP64,
	GPU_SET,
	GPU_SET64,
	GPU_SETP,
	GPU_SETP64,
	GPU_SHL,
	GPU_SHL64,
	GPU_SHR,
	GPU_SHR64,
	GPU_SIN,
	GPU_SLCT,
	GPU_SLCT64,
	GPU_SQRT,
	GPU_SQRT64,
	GPU_ST,
	GPU_ST64,
	GPU_SUB,
	GPU_SUB64,
	GPU_SUBC,
	GPU_SULD,
	GPU_SULD64,
	GPU_SURED,
	GPU_SURED64,
	GPU_SUST,
	GPU_SUST64,
	GPU_SUQ,
  GPU_TESTP,
  GPU_TESTP64,
  GPU_TEX,
  GPU_TLD4,
  GPU_TXQ,
  GPU_TRAP,
  GPU_VABSDIFF,
  GPU_VADD,
  GPU_VMAD,
  GPU_VMAX,
  GPU_VMIN,
  GPU_VSET,
  GPU_VSHL,
  GPU_VSHR,
  GPU_VSUB,
  GPU_VOTE,
  GPU_XOR,
  GPU_XOR64,
  GPU_RECONVERGE,
  GPU_PHI,
  TR_OPCODE_LAST
} TR_OPCODE_ENUM;


// Sanity check::
// We should match with total number of instructions in gpuocelot (ptx)
//
// index 0 for integer instruction
// index 1 for fp instruction

#ifndef GPU_VALIDATION

int ptx_to_x86[2][ir::PTXInstruction::Nop] = 
{
  {
    (int)XED_CATEGORY_DECIMAL,  //Abs 
    (int)XED_CATEGORY_DECIMAL,  //Add - only int
    (int)XED_CATEGORY_DECIMAL,  //AddC - only int
    (int)XED_CATEGORY_LOGICAL,  // And
    (int)XED_CATEGORY_DATAXFER, //Atom
    (int)XED_CATEGORY_SEMAPHORE, //Bar
    (int)XED_CATEGORY_DECIMAL,  // ** Bfe int
    (int)XED_CATEGORY_DECIMAL,  // ** Bfi int 
    (int)XED_CATEGORY_DECIMAL,  // ** Bfind int
    (int)XED_CATEGORY_COND_BR,  //Bra
    (int)XED_CATEGORY_DECIMAL,  // ** Brev int
    (int)XED_CATEGORY_DECIMAL,  // ** Brkpt misc
    (int)XED_CATEGORY_CALL,     //Call
    (int)XED_CATEGORY_DECIMAL,  // ** Clz int
    (int)XED_CATEGORY_DECIMAL,  //CNot,
    (int)XED_CATEGORY_DECIMAL,  // copysign
    (int)TR_FMUL,               //Cos - only fp
    (int)XED_CATEGORY_DATAXFER, // ** Cvt was TR_MUL
    (int)XED_CATEGORY_DATAXFER, // ** Cvta mov
    (int)TR_DIV,                //Div
    (int)TR_FDIV,               //Ex2 - only fp
    (int)XED_CATEGORY_DECIMAL,  //Exit
    (int)TR_FMUL,               // fma
    (int)XED_CATEGORY_DECIMAL,  // isspacep
    (int)XED_CATEGORY_DATAXFER, //Ld
    (int)XED_CATEGORY_DATAXFER, // ** Ldu mov
    (int)TR_FDIV,               //Lg2 - only fp
    (int)TR_MUL,                //Mad24 - only int
    (int)TR_MUL,                //Mad
    (int)XED_CATEGORY_DECIMAL,  //Max,
    (int)XED_CATEGORY_SEMAPHORE, //Membar,
    (int)XED_CATEGORY_DECIMAL,  //Min,
    (int)XED_CATEGORY_DATAXFER, //Mov,
    (int)TR_MUL,                //Mul24 - only int
    (int)TR_MUL,                //Mul
    (int)XED_CATEGORY_LOGICAL,  //Neg
    (int)XED_CATEGORY_LOGICAL,  //Not
    (int)XED_CATEGORY_LOGICAL,  // Or
    (int)XED_CATEGORY_LOGICAL,  //Pmevent
    (int)XED_CATEGORY_DECIMAL,  // ** Popc int
    (int)XED_CATEGORY_DATAXFER, // ** Prefetch mov
    (int)XED_CATEGORY_DATAXFER, // ** Prefetchu mov
    (int)XED_CATEGORY_DECIMAL,  // ** Prmt int
    (int)TR_FDIV,               //Rcp - only fp
    (int)XED_CATEGORY_DATAXFER, //Red
    (int)TR_DIV,                //Rem - only int
    (int)XED_CATEGORY_RET,      //Ret
    (int)TR_FDIV,               //Rsqrt
    (int)TR_MUL,                //Sad - only int
    (int)XED_CATEGORY_DECIMAL,  //SelP
    (int)XED_CATEGORY_DECIMAL,  //Set
    (int)XED_CATEGORY_DECIMAL,  //SetP
    (int)XED_CATEGORY_SHIFT,    //Shl
    (int)XED_CATEGORY_SHIFT,    //Shr
    (int)TR_FMUL,               //Sin - only float
    (int)XED_CATEGORY_DECIMAL,  //SlCt
    (int)TR_FDIV,               //Sqrt
    (int)XED_CATEGORY_DATAXFER, //St
    (int)XED_CATEGORY_DECIMAL,  //Sub
    (int)XED_CATEGORY_DECIMAL,  //SubC
    (int)XED_CATEGORY_DATAXFER, // ** Suld surface mem
    (int)XED_CATEGORY_DATAXFER, // ** Sured surface mem
    (int)XED_CATEGORY_DATAXFER, // ** Sust surface mem
    (int)XED_CATEGORY_DATAXFER, // ** Suq surface mem
    (int)XED_CATEGORY_LOGICAL,  // TestP
    (int)XED_CATEGORY_DATAXFER, //Tex
    (int)XED_CATEGORY_DATAXFER, // Tld4
    (int)XED_CATEGORY_DATAXFER, // ** Txq texture mem
    (int)XED_CATEGORY_INTERRUPT, //Trap
    (int)XED_CATEGORY_DECIMAL,  // ** Vadsdiff video
    (int)XED_CATEGORY_DECIMAL,  // ** Vadd video 
    (int)TR_MUL,                // ** Vmad video
    (int)XED_CATEGORY_DECIMAL,  // ** Vmax video
    (int)XED_CATEGORY_DECIMAL,  // ** Vmin video
    (int)XED_CATEGORY_DECIMAL,  // ** Vset video
    (int)XED_CATEGORY_SHIFT,    // ** Vshl video
    (int)XED_CATEGORY_SHIFT,    // ** Vshr video
    (int)XED_CATEGORY_DECIMAL,  // ** Vsub video
    (int)XED_CATEGORY_DATAXFER, //Vote, - check this
    (int)XED_CATEGORY_LOGICAL,  //Xor,
    (int)XED_CATEGORY_DECIMAL,  //Reconverge, - check
    (int)XED_CATEGORY_DECIMAL,  //, //Phi - check
//    (int)XED_CATEGORY_DECIMAL   //Nop
  },
  {
    (int)XED_CATEGORY_DECIMAL,  //Abs 
    (int)XED_CATEGORY_DECIMAL,  //Add - only int
    (int)XED_CATEGORY_DECIMAL,  //AddC - only int
    (int)XED_CATEGORY_LOGICAL,  // And
    (int)XED_CATEGORY_DATAXFER, //Atom
    (int)XED_CATEGORY_SEMAPHORE, //Bar
    (int)XED_CATEGORY_DECIMAL,  // ** Bfe int
    (int)XED_CATEGORY_DECIMAL,  // ** Bfi int 
    (int)XED_CATEGORY_DECIMAL,  // ** Bfind int
    (int)XED_CATEGORY_COND_BR,  //Bra
    (int)XED_CATEGORY_DECIMAL,  // ** Brev int
    (int)XED_CATEGORY_DECIMAL,  //Brkpt
    (int)XED_CATEGORY_CALL,     //Call
    (int)XED_CATEGORY_DECIMAL,  // ** Clz int
    (int)XED_CATEGORY_DECIMAL,  //CNot,
    (int)XED_CATEGORY_DECIMAL,  // copysign
    (int)TR_FMUL,               //Cos - only fp
    (int)XED_CATEGORY_DATAXFER, // ** Cvt was TR_MUL
    (int)XED_CATEGORY_DATAXFER, // ** Cvta mov
    (int)TR_FDIV,               //Div
    (int)TR_FDIV,               //Ex2 - only fp
    (int)XED_CATEGORY_DECIMAL,  //Exit
    (int)TR_MUL,                // fma
    (int)XED_CATEGORY_LOGICAL,  // isspacep
    (int)XED_CATEGORY_DATAXFER, //Ld
    (int)XED_CATEGORY_DATAXFER, // ** Ldu mov
    (int)TR_FDIV,               //Lg2 - only fp
    (int)TR_MUL,                //Mad24 - only int
    (int)TR_MUL,                //Mad
    (int)XED_CATEGORY_DECIMAL,  //Max,
    (int)XED_CATEGORY_SEMAPHORE, //Membar,
    (int)XED_CATEGORY_DECIMAL,  //Min,
    (int)XED_CATEGORY_DATAXFER, //Mov,
    (int)TR_MUL,                //Mul24 - only int
    (int)TR_MUL,                //Mul
    (int)XED_CATEGORY_LOGICAL,  //Neg
    (int)XED_CATEGORY_LOGICAL,  //Not
    (int)XED_CATEGORY_LOGICAL,  // Or
    (int)XED_CATEGORY_LOGICAL,  //Pmevent
    (int)XED_CATEGORY_DECIMAL,  // ** Popc int
    (int)XED_CATEGORY_DATAXFER, // ** Prefetch mov
    (int)XED_CATEGORY_DATAXFER, // ** Prefetchu mov
    (int)XED_CATEGORY_DECIMAL,  // ** Prmt int
    (int)TR_FDIV,               //Rcp
    (int)XED_CATEGORY_DATAXFER, //Red
    (int)TR_DIV,                //Rem - only int
    (int)XED_CATEGORY_RET,      //Ret
    (int)TR_FDIV,               //Rsqrt
    (int)TR_MUL,                //Sad - only int
    (int)XED_CATEGORY_DECIMAL,  //SelP
    (int)XED_CATEGORY_DECIMAL,  //Set
    (int)XED_CATEGORY_DECIMAL,  //SetP
    (int)XED_CATEGORY_SHIFT,    //Shl
    (int)XED_CATEGORY_SHIFT,    //Shr
    (int)TR_FMUL,               //Sin - only float
    (int)XED_CATEGORY_DECIMAL,  //SlCt
    (int)TR_FDIV,               //Sqrt
    (int)XED_CATEGORY_DATAXFER, //St
    (int)XED_CATEGORY_DECIMAL,  //Sub
    (int)XED_CATEGORY_DECIMAL,  //SubC
    (int)XED_CATEGORY_DATAXFER, // ** Suld surface mem
    (int)XED_CATEGORY_DATAXFER, // ** Sured surface mem
    (int)XED_CATEGORY_DATAXFER, // ** Sust surface mem
    (int)XED_CATEGORY_DATAXFER, // ** Suq surface mem
    (int)XED_CATEGORY_LOGICAL,  // TestP
    (int)XED_CATEGORY_DATAXFER, //Tex
    (int)XED_CATEGORY_DATAXFER, // Tld4
    (int)XED_CATEGORY_DATAXFER, // ** Txq texture mem
    (int)XED_CATEGORY_INTERRUPT, //Trap
    (int)XED_CATEGORY_DECIMAL,  // ** Vadsdiff video
    (int)XED_CATEGORY_DECIMAL,  // ** Vadd video 
    (int)TR_FMUL,               // ** Vmad video
    (int)XED_CATEGORY_DECIMAL,  // ** Vmax video
    (int)XED_CATEGORY_DECIMAL,  // ** Vmin video
    (int)XED_CATEGORY_DECIMAL,  // ** Vset video
    (int)XED_CATEGORY_SHIFT,    // ** Vshl video
    (int)XED_CATEGORY_SHIFT,    // ** Vshr video
    (int)XED_CATEGORY_DECIMAL,  // ** Vsub video
    (int)XED_CATEGORY_DATAXFER, //Vote, - check this
    (int)XED_CATEGORY_LOGICAL,  //Xor,
    (int)XED_CATEGORY_DECIMAL,  //Reconverge, - check
    (int)XED_CATEGORY_DECIMAL,   //, //Phi - check
//    (int)XED_CATEGORY_DECIMAL //Nop
  }
};

#else

int ptx_to_x86[4][ir::PTXInstruction::Nop] = 
{
  // int (upto) 32-bit
  {
			(int)GPU_ABS,         // Abs
			(int)GPU_ADD,         // Add
			(int)GPU_ADDC,        // AddC
			(int)GPU_AND,         // And
			(int)GPU_ATOM,        // Atom
			(int)GPU_BAR,         // Bar
			(int)GPU_BFE,         // Bfe
			(int)GPU_BFI,         // Bfi
			(int)GPU_BFIND,       // Bfind
			(int)GPU_BRA,         // Bra
			(int)GPU_BREV,        // Brev
			(int)GPU_BRKPT,       // Brkpt
			(int)GPU_CALL,        // Call
			(int)GPU_CLZ,         // Clz
			(int)GPU_CNOT,        // CNot
			(int)XED_CATEGORY_INVALID,    // CopySign
			(int)XED_CATEGORY_INVALID,         // Cos
			(int)GPU_CVT,         // Cvt
			(int)GPU_CVTA,        // Cvta
			(int)GPU_DIV,         // Div
			(int)XED_CATEGORY_INVALID,         // Ex2
			(int)GPU_EXIT,        // Exit
			(int)XED_CATEGORY_INVALID,         // Fma
			(int)GPU_ISSPACEP,    // Isspacep
			(int)GPU_LD,          // Ld
			(int)GPU_LDU,         // Ldu
			(int)XED_CATEGORY_INVALID,         // Lg2
			(int)GPU_MAD24,       // Mad24
			(int)GPU_MAD,         // Mad
			(int)GPU_MAX,         // Max
			(int)GPU_MEMBAR,      // Membar
			(int)GPU_MIN,         // Min
			(int)GPU_MOV,         // Mov
			(int)GPU_MUL24,       // Mul24
			(int)GPU_MUL,         // Mul
			(int)GPU_NEG,         // Neg
			(int)GPU_NOT,         // Not
			(int)GPU_OR,          // Or
			(int)GPU_PMEVENT,     // Pmevent
			(int)GPU_POPC,        // Popc
			(int)GPU_PREFETCH,    // Prefetch
			(int)GPU_PREFETCHU,   // Prefetchu
			(int)GPU_PRMT,        // Prmt
			(int)XED_CATEGORY_INVALID,         // Rcp
			(int)GPU_RED,         // Red
			(int)GPU_REM,         // Rem
			(int)GPU_RET,         // Ret
			(int)XED_CATEGORY_INVALID,       // Rsqrt
			(int)GPU_SAD,         // Sad
			(int)GPU_SELP,        // SelP
			(int)GPU_SET,         // Set
			(int)GPU_SETP,        // SetP
			(int)GPU_SHL,         // Shl
			(int)GPU_SHR,         // Shr
			(int)XED_CATEGORY_INVALID,         // Sin
			(int)GPU_SLCT,        // SlCt
			(int)XED_CATEGORY_INVALID,        // Sqrt
			(int)GPU_ST,          // St
			(int)GPU_SUB,         // Sub
			(int)GPU_SUBC,        // SubC
			(int)XED_CATEGORY_INVALID,        // Suld
			(int)XED_CATEGORY_INVALID,       // Sured
			(int)XED_CATEGORY_INVALID,        // Sust
			(int)XED_CATEGORY_INVALID,         // Suq
			(int)XED_CATEGORY_INVALID,       // TestP
			(int)XED_CATEGORY_INVALID,         // Tex
			(int)XED_CATEGORY_INVALID,        // Tld4
			(int)XED_CATEGORY_INVALID,         // Txq
			(int)XED_CATEGORY_INVALID,        // Trap
			(int)XED_CATEGORY_INVALID,    // Vabsdiff
			(int)XED_CATEGORY_INVALID,        // Vadd
			(int)XED_CATEGORY_INVALID,        // Vmad
			(int)XED_CATEGORY_INVALID,        // Vmax
			(int)XED_CATEGORY_INVALID,        // Vmin
			(int)XED_CATEGORY_INVALID,        // Vset
			(int)XED_CATEGORY_INVALID,        // Vshl
			(int)XED_CATEGORY_INVALID,        // Vshr
			(int)XED_CATEGORY_INVALID,        // Vsub
			(int)GPU_VOTE,        // Vote
			(int)GPU_XOR,         // Xor
			(int)GPU_RECONVERGE,  // Reconverge
			(int)XED_CATEGORY_INVALID,         // Phi
  },
  // int 64-bit
  {
			(int)GPU_ABS64,         // Abs
			(int)GPU_ADD64,         // Add
			(int)XED_CATEGORY_INVALID,        // AddC
			(int)GPU_AND64,         // And
			(int)GPU_ATOM64,        // Atom
			(int)GPU_BAR,           // Bar *************** bar type is b64, so not set to invalid
			(int)GPU_BFE64,         // Bfe
			(int)GPU_BFI64,         // Bfi
			(int)GPU_BFIND64,       // Bfind
			(int)GPU_BRA,           // Bra *************** bra type is b64, so not set to invalid
			(int)GPU_BREV64,        // Brev
			(int)XED_CATEGORY_INVALID,       // Brkpt
			(int)XED_CATEGORY_INVALID,        // Call
			(int)GPU_CLZ64,         // Clz
			(int)GPU_CNOT64,        // CNot
			(int)XED_CATEGORY_INVALID,    // CopySign
			(int)XED_CATEGORY_INVALID,         // Cos
			(int)GPU_CVT64,         // Cvt
			(int)GPU_CVTA64,        // Cvta
			(int)GPU_DIV64,         // Div
			(int)XED_CATEGORY_INVALID,         // Ex2
			(int)GPU_EXIT,          // Exit ************* exit type is b64, so not set to invalid
			(int)XED_CATEGORY_INVALID,         // Fma
			(int)XED_CATEGORY_INVALID,    // Isspacep
			(int)GPU_LD64,          // Ld
			(int)GPU_LDU64,         // Ldu
			(int)XED_CATEGORY_INVALID,         // Lg2
			(int)XED_CATEGORY_INVALID,       // Mad24
			(int)GPU_MAD64,         // Mad
			(int)GPU_MAX64,         // Max
			(int)XED_CATEGORY_INVALID,      // Membar
			(int)GPU_MIN64,         // Min
			(int)GPU_MOV64,         // Mov
			(int)XED_CATEGORY_INVALID,       // Mul24
			(int)GPU_MUL64,         // Mul
			(int)GPU_NEG64,         // Neg
			(int)GPU_NOT64,         // Not
			(int)GPU_OR64,          // Or
			(int)XED_CATEGORY_INVALID,     // Pmevent
			(int)GPU_POPC64,        // Popc
			(int)XED_CATEGORY_INVALID,    // Prefetch
			(int)XED_CATEGORY_INVALID,   // Prefetchu
			(int)XED_CATEGORY_INVALID,        // Prmt
			(int)GPU_RCP64,         // Rcp
			(int)GPU_RED64,         // Red
			(int)GPU_REM64,         // Rem
			(int)XED_CATEGORY_INVALID,         // Ret
			(int)XED_CATEGORY_INVALID,       // Rsqrt
			(int)GPU_SAD64,         // Sad
			(int)GPU_SELP64,        // SelP
			(int)GPU_SET64,         // Set
			(int)GPU_SETP64,        // SetP
			(int)GPU_SHL64,         // Shl
			(int)GPU_SHR64,         // Shr
			(int)XED_CATEGORY_INVALID,         // Sin
			(int)GPU_SLCT64,        // SlCt
			(int)XED_CATEGORY_INVALID,        // Sqrt
			(int)GPU_ST64,          // St
			(int)GPU_SUB64,         // Sub
			(int)XED_CATEGORY_INVALID,        // SubC
			(int)XED_CATEGORY_INVALID,        // Suld
			(int)XED_CATEGORY_INVALID,       // Sured
			(int)XED_CATEGORY_INVALID,        // Sust
			(int)XED_CATEGORY_INVALID,         // Suq
			(int)XED_CATEGORY_INVALID,       // TestP
			(int)XED_CATEGORY_INVALID,         // Tex
			(int)XED_CATEGORY_INVALID,        // Tld4
			(int)XED_CATEGORY_INVALID,         // Txq
			(int)XED_CATEGORY_INVALID,        // Trap
			(int)XED_CATEGORY_INVALID,    // Vabsdiff
			(int)XED_CATEGORY_INVALID,        // Vadd
			(int)XED_CATEGORY_INVALID,        // Vmad
			(int)XED_CATEGORY_INVALID,        // Vmax
			(int)XED_CATEGORY_INVALID,        // Vmin
			(int)XED_CATEGORY_INVALID,        // Vset
			(int)XED_CATEGORY_INVALID,        // Vshl
			(int)XED_CATEGORY_INVALID,        // Vshr
			(int)XED_CATEGORY_INVALID,        // Vsub
			(int)XED_CATEGORY_INVALID,        // Vote
			(int)GPU_XOR64,         // Xor
			(int)XED_CATEGORY_INVALID,  // Reconverge
			(int)XED_CATEGORY_INVALID,         // Phi
  },
  // FP (32-bit)
  {
			(int)GPU_ABS,         // Abs
			(int)GPU_ADD,         // Add
			(int)XED_CATEGORY_INVALID,        // AddC
			(int)XED_CATEGORY_INVALID,         // And
			(int)GPU_ATOM,        // Atom
			(int)XED_CATEGORY_INVALID,         // Bar
			(int)XED_CATEGORY_INVALID,         // Bfe
			(int)XED_CATEGORY_INVALID,         // Bfi
			(int)XED_CATEGORY_INVALID,       // Bfind
			(int)XED_CATEGORY_INVALID,         // Bra
			(int)XED_CATEGORY_INVALID,        // Brev
			(int)XED_CATEGORY_INVALID,       // Brkpt
			(int)XED_CATEGORY_INVALID,        // Call
			(int)XED_CATEGORY_INVALID,         // Clz
			(int)XED_CATEGORY_INVALID,        // CNot
			(int)GPU_COPYSIGN,    // CopySign
			(int)GPU_COS,         // Cos
			(int)GPU_CVT,         // Cvt
			(int)XED_CATEGORY_INVALID,        // Cvta
			(int)GPU_DIV,         // Div
			(int)GPU_EX2,         // Ex2
			(int)XED_CATEGORY_INVALID,        // Exit
			(int)GPU_FMA,         // Fma
			(int)XED_CATEGORY_INVALID,    // Isspacep
			(int)GPU_LD,          // Ld
			(int)GPU_LDU,         // Ldu
			(int)GPU_LG2,         // Lg2
			(int)XED_CATEGORY_INVALID,       // Mad24
			(int)GPU_MAD,         // Mad
			(int)GPU_MAX,         // Max
			(int)XED_CATEGORY_INVALID,      // Membar
			(int)GPU_MIN,         // Min
			(int)GPU_MOV,         // Mov
			(int)XED_CATEGORY_INVALID,       // Mul24
			(int)GPU_MUL,         // Mul
			(int)GPU_NEG,         // Neg
			(int)XED_CATEGORY_INVALID,         // Not
			(int)XED_CATEGORY_INVALID,          // Or
			(int)XED_CATEGORY_INVALID,     // Pmevent
			(int)XED_CATEGORY_INVALID,        // Popc
			(int)XED_CATEGORY_INVALID,    // Prefetch
			(int)XED_CATEGORY_INVALID,   // Prefetchu
			(int)XED_CATEGORY_INVALID,        // Prmt
			(int)GPU_RCP,         // Rcp
			(int)GPU_RED,         // Red
			(int)XED_CATEGORY_INVALID,         // Rem
			(int)XED_CATEGORY_INVALID,         // Ret
			(int)GPU_RSQRT,       // Rsqrt
			(int)XED_CATEGORY_INVALID,         // Sad
			(int)GPU_SELP,        // SelP
			(int)GPU_SET,         // Set
			(int)GPU_SETP,        // SetP
			(int)XED_CATEGORY_INVALID,         // Shl
			(int)XED_CATEGORY_INVALID,         // Shr
			(int)GPU_SIN,         // Sin
			(int)GPU_SLCT,        // SlCt
			(int)GPU_SQRT,        // Sqrt
			(int)GPU_ST,          // St
			(int)GPU_SUB,         // Sub
			(int)XED_CATEGORY_INVALID,        // SubC
			(int)XED_CATEGORY_INVALID,        // Suld
			(int)XED_CATEGORY_INVALID,       // Sured
			(int)XED_CATEGORY_INVALID,        // Sust
			(int)XED_CATEGORY_INVALID,         // Suq
			(int)XED_CATEGORY_INVALID,       // TestP
			(int)XED_CATEGORY_INVALID,         // Tex
			(int)XED_CATEGORY_INVALID,        // Tld4
			(int)XED_CATEGORY_INVALID,         // Txq
			(int)XED_CATEGORY_INVALID,        // Trap
			(int)XED_CATEGORY_INVALID,    // Vabsdiff
			(int)XED_CATEGORY_INVALID,        // Vadd
			(int)XED_CATEGORY_INVALID,        // Vmad
			(int)XED_CATEGORY_INVALID,        // Vmax
			(int)XED_CATEGORY_INVALID,        // Vmin
			(int)XED_CATEGORY_INVALID,        // Vset
			(int)XED_CATEGORY_INVALID,        // Vshl
			(int)XED_CATEGORY_INVALID,        // Vshr
			(int)XED_CATEGORY_INVALID,        // Vsub
			(int)XED_CATEGORY_INVALID,        // Vote
			(int)XED_CATEGORY_INVALID,         // Xor
			(int)XED_CATEGORY_INVALID,  // Reconverge
			(int)XED_CATEGORY_INVALID,         // Phi
  },
  // double (64-bit)
  {
			(int)GPU_ABS64,         // Abs
			(int)GPU_ADD64,         // Add
			(int)XED_CATEGORY_INVALID,        // AddC
			(int)XED_CATEGORY_INVALID,         // And
			(int)XED_CATEGORY_INVALID,        // Atom
			(int)XED_CATEGORY_INVALID,         // Bar
			(int)XED_CATEGORY_INVALID,         // Bfe
			(int)XED_CATEGORY_INVALID,         // Bfi
			(int)XED_CATEGORY_INVALID,       // Bfind
			(int)XED_CATEGORY_INVALID,         // Bra
			(int)XED_CATEGORY_INVALID,        // Brev
			(int)XED_CATEGORY_INVALID,       // Brkpt
			(int)XED_CATEGORY_INVALID,        // Call
			(int)XED_CATEGORY_INVALID,         // Clz
			(int)XED_CATEGORY_INVALID,        // CNot
			(int)GPU_COPYSIGN64,    // CopySign
			(int)XED_CATEGORY_INVALID,         // Cos
			(int)GPU_CVT64,         // Cvt
			(int)XED_CATEGORY_INVALID,        // Cvta
			(int)GPU_DIV64,         // Div
			(int)XED_CATEGORY_INVALID,         // Ex2
			(int)XED_CATEGORY_INVALID,        // Exit
			(int)GPU_FMA64,         // Fma
			(int)XED_CATEGORY_INVALID,    // Isspacep
			(int)GPU_LD64,          // Ld
			(int)GPU_LDU64,         // Ldu
			(int)XED_CATEGORY_INVALID,         // Lg2
			(int)XED_CATEGORY_INVALID,       // Mad24
			(int)GPU_MAD64,         // Mad
			(int)GPU_MAX64,         // Max
			(int)XED_CATEGORY_INVALID,      // Membar
			(int)GPU_MIN64,         // Min
			(int)GPU_MOV64,         // Mov
			(int)XED_CATEGORY_INVALID,       // Mul24
			(int)GPU_MUL64,         // Mul
			(int)GPU_NEG64,         // Neg
			(int)XED_CATEGORY_INVALID,         // Not
			(int)XED_CATEGORY_INVALID,          // Or
			(int)XED_CATEGORY_INVALID,     // Pmevent
			(int)XED_CATEGORY_INVALID,        // Popc
			(int)XED_CATEGORY_INVALID,    // Prefetch
			(int)XED_CATEGORY_INVALID,   // Prefetchu
			(int)XED_CATEGORY_INVALID,        // Prmt
			(int)GPU_RCP64,         // Rcp
			(int)XED_CATEGORY_INVALID,         // Red
			(int)XED_CATEGORY_INVALID,         // Rem
			(int)XED_CATEGORY_INVALID,         // Ret
			(int)GPU_RSQRT64,       // Rsqrt
			(int)XED_CATEGORY_INVALID,         // Sad
			(int)GPU_SELP64,        // SelP
			(int)GPU_SET64,         // Set
			(int)GPU_SETP64,        // SetP
			(int)XED_CATEGORY_INVALID,         // Shl
			(int)XED_CATEGORY_INVALID,         // Shr
			(int)XED_CATEGORY_INVALID,         // Sin
			(int)GPU_SLCT64,        // SlCt
			(int)GPU_SQRT64,        // Sqrt
			(int)GPU_ST64,          // St
			(int)GPU_SUB64,         // Sub
			(int)XED_CATEGORY_INVALID,        // SubC
			(int)XED_CATEGORY_INVALID,        // Suld
			(int)XED_CATEGORY_INVALID,       // Sured
			(int)XED_CATEGORY_INVALID,        // Sust
			(int)XED_CATEGORY_INVALID,         // Suq
			(int)GPU_TESTP64,       // TestP
			(int)XED_CATEGORY_INVALID,         // Tex
			(int)XED_CATEGORY_INVALID,        // Tld4
			(int)XED_CATEGORY_INVALID,         // Txq
			(int)XED_CATEGORY_INVALID,        // Trap
			(int)XED_CATEGORY_INVALID,    // Vabsdiff
			(int)XED_CATEGORY_INVALID,        // Vadd
			(int)XED_CATEGORY_INVALID,        // Vmad
			(int)XED_CATEGORY_INVALID,        // Vmax
			(int)XED_CATEGORY_INVALID,        // Vmin
			(int)XED_CATEGORY_INVALID,        // Vset
			(int)XED_CATEGORY_INVALID,        // Vshl
			(int)XED_CATEGORY_INVALID,        // Vshr
			(int)XED_CATEGORY_INVALID,        // Vsub
			(int)XED_CATEGORY_INVALID,        // Vote
			(int)XED_CATEGORY_INVALID,         // Xor
			(int)XED_CATEGORY_INVALID,  // Reconverge
			(int)XED_CATEGORY_INVALID,         // Phi
  }
};

#endif


///////////////////////////////////////////////////////////////////////////////////////////////
// Control Flow Instruction enumerator
// These should be exactly matched with the Macsim
///////////////////////////////////////////////////////////////////////////////////////////////
enum CF_TYPE_enum {
  NOT_CF,                       // not a control flow instruction
  CF_BR,                        // an unconditional branch
  CF_CBR,                       // a conditional branch
  CF_CALL,                      // a call
  // below this point are indirect cfs
  CF_IBR,                       // an indirect branch
  CF_ICALL,                     // an indirect call
  CF_ICO,                       // an indirect jump to co-routine
  CF_RET,                       // a return
  CF_SYS,
  CF_ICBR                       // an indirect conditional branch
} CF_TYPE;


///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////



#if WRITE_DEBUG == 1
ofstream *debug_stream;
#endif



///////////////////////////////////////////////////////////////////////////////////////////////
// We need to set environmental variables
// TRACE_PATH
// TRACE_NAME
// USE_KERNEL_NAME


///////////////////////////////////////////////////////////////////////////////////////////////
// Ptx to X86 trace generator constructor
///////////////////////////////////////////////////////////////////////////////////////////////
trace::X86TraceGenerator::X86TraceGenerator()
  : init(false)
  , num_warps_per_block(0)
  , num_total_warps(0)
  , blockDimX(1)
  , blockDimY(1)
  , blockDimZ(1)
  , gridDimX(1)
  , gridDimY(1)
  , gridDimZ(1)
  , kernel_count(0)
  , total_inst_count(0)
{
  m_compute = "2.0";
  mem_addr_flag = ~0u;

}


///////////////////////////////////////////////////////////////////////////////////////////////
// destructor
// Ptx to X86 trace generator
///////////////////////////////////////////////////////////////////////////////////////////////
trace::X86TraceGenerator::~X86TraceGenerator()
{
  report("destructor");
  finalize();

  if (init) {
#if WRITE_DEBUG == 1
    debug_stream->close();
    delete debug_stream;
#endif
    txt_kernel_config_file.close();
  }
}


int trace::X86TraceGenerator::calculate_occupancy(const executive::ExecutableKernel& kernel)
{
  ///
  /// Kernel information
  ///
  blockDimX = kernel.blockDim().x;
  blockDimY = kernel.blockDim().y;
  blockDimZ = kernel.blockDim().z;
  gridDimX  = kernel.gridDim().x;
  gridDimY  = kernel.gridDim().y;
  gridDimZ  = kernel.gridDim().z; // will be 1 always

  assert(gridDimZ == 1);

  num_warps_per_block = (blockDimX * blockDimY * blockDimZ + (WARP_SIZE - 1)) / WARP_SIZE;
  num_total_warps = num_warps_per_block * gridDimX * gridDimY * gridDimZ;


  // maximum block calculation
  int max_block = 8;


  // base hardware configuration (2.0 compute capability)
  int total_shared_memory = 49152;
  int total_thread        = 1536;
  int total_register      = 32768;
  int shared_mem_allocation_granularity = 128;


  // Registers calculation
  int num_register_per_block  = 0;
  int num_register_per_thread = m_kernel_register_map[kernel.name];
  int shared_mem_per_block    = m_kernel_sharedmem_map[kernel.name];


  report("New kernel launched");
  report("compute version:" << m_compute);
  report("grid  " << gridDimX <<  " x " << gridDimY <<  " x " << gridDimZ);
  report("block " << blockDimX << " x " << blockDimY << " x " << blockDimZ);
  report("number of warps per block:" << num_warps_per_block);
  report("number of total warps:" << num_total_warps);
  report("# threads per block : " << num_warps_per_block * WARP_SIZE);
  report("number of register per thread:" << num_register_per_thread);
  report("number of shared memory per thread:" << shared_mem_per_block);


  // Threads calculation
  int num_threads_per_block = num_warps_per_block * WARP_SIZE;
  if (max_block > total_thread / num_threads_per_block)
    max_block = total_thread / num_threads_per_block;

  // Shared memory calculation
  if (shared_mem_per_block > 0) {
    int ceil_1 = ((shared_mem_per_block + (shared_mem_allocation_granularity - 1)) /
        shared_mem_allocation_granularity) * shared_mem_allocation_granularity;

    if (ceil_1 == 0)
      ceil_1 = shared_mem_allocation_granularity;

    if (max_block > total_shared_memory / ceil_1)
      max_block = total_shared_memory / ceil_1;
  }


  if (m_compute == "2.0") {
    int ceil_1 = num_register_per_thread * 32;
    ceil_1 = ((ceil_1 + (64-1)) / 64) * 64;
    if (ceil_1 == 0)
      ceil_1 = 64;
    ceil_1 *= num_warps_per_block;
    num_register_per_block = ceil_1;
  }
  // TODO (jaekyu, 9-26-2012)
  // To support other computing capability
  else {
    assert(0);
  }

  if (max_block > total_register / num_register_per_block)
    max_block = total_register / num_register_per_block;

  report("max blocks per core : " << max_block);

  return max_block;
}


///////////////////////////////////////////////////////////////////////////////////////////////
// called when a traced kernel is launched to retrieve some parameters from the kernel
///////////////////////////////////////////////////////////////////////////////////////////////
void trace::X86TraceGenerator::initialize(const executive::ExecutableKernel& kernel)
{
  ///
  /// Initialization of GPUOcelot-Macsim trace generator
  ///
  static bool init_x86_tracegenerator = false;
  if (!init_x86_tracegenerator) {
    // We have opcodes for 82 GPU Instructions! this check lets us 
    // know if new instructions have been added (or removed), but 
    // what if there have been modifications and count hasn't changed?
    assert(ir::PTXInstruction::Nop == 82); 

    init_x86_tracegenerator = true;

    char *t_name;
    char *t_path;

    // get TRACE_PATH environment variable
    t_path = getenv("TRACE_PATH");
    if (t_path == NULL) {
      report("TRACE_PATH not set up");
    }
    // assert(t_path != NULL);

    trace_path = t_path;
    trace_path.append("/");


    // get TRACE_NAME environment variable
    t_name = getenv("TRACE_NAME");
    if (t_name != NULL)
      trace_name = t_name;
    else
      trace_name = "Trace";


    // get KERNEL_INFO_PATH : get register usage information from the file
    char* kernel_info_path = getenv("KERNEL_INFO_PATH");
    ifstream kernel_info_file;

    kernel_info_file.open(kernel_info_path);

    string kernel_name;
    int register_num;
    int sharedmem;

    if (!kernel_info_file.fail()) {
      while (kernel_info_file >> kernel_name >> register_num >> sharedmem) {
        m_kernel_register_map[kernel_name] = register_num;
        m_kernel_sharedmem_map[kernel_name] = sharedmem;
      }
    }
    else {
    }

    // get compute version
    char* compute_version = getenv("COMPUTE_VERSION");
    if (compute_version != NULL)
      m_compute = string(compute_version);


    // set trace file extension
    extension = "raw";

    last_block_id = -1;
  }


  /////////////////////////////////////////////////////////////////////////////////////////////
  // If multiple kernels exist in the program, initialize() function will be called several 
  // times. There are some structures that are used in the previous kernel
  // thus, we need to reset those values
  /////////////////////////////////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////////////////////////////////
  // Since each block is executed in sequential, we open gzFile for current block and
  // close when it has been terminate. However, since we don't know when exactly a block
  // terminates, we maintain last_block_id and compare with current block id.
  // When ids do not match, a new block has been started.
  /////////////////////////////////////////////////////////////////////////////////////////////
  if (last_block_id != -1) {
    int block_id = last_block_id;
    for (int ii = 0; ii < num_warps_per_block; ++ii) {
      Trace_info *tr_info = &trace_info[block_id * num_warps_per_block + ii];
      if (tr_info->bytes_accumulated) {
        int bytes_written = gzwrite(tr_info->trace_stream, tr_info->trace_buf, tr_info->bytes_accumulated); 
        if (bytes_written != tr_info->bytes_accumulated) {
          report("gz file write error");
          assert(0);
        }
      }
      gzclose(tr_info->trace_stream);

      Thread_info *th_info = &thread_info[block_id * num_warps_per_block + ii];
      if (sizeof(Thread_info) != gzwrite(gz_config_file, th_info, sizeof(Thread_info))) {
        report("unable to write to config file");
        exit(-1);
      }

      txt_config_file << th_info->thread_id << " " << th_info->inst_count << "\n";
      info_file << th_info->thread_id << " " << th_info->inst_count << "\n";
    }

    txt_config_file.close();
    info_file.close();
  }

  /////////////////////////////////////////////////////////////////////////////////////////////
  // Termination condition check
  /////////////////////////////////////////////////////////////////////////////////////////////
  if (total_inst_count > 100000000) {
    report("total instruction count exceeds 100M.. stop here...");
    assert(0);
    exit(0);
  }


  /////////////////////////////////////////////////////////////////////////////////////////////
  /// reset instruction hash for a new kernel
  /////////////////////////////////////////////////////////////////////////////////////////////
  for (auto itr = inst_storage.begin(); itr != inst_storage.end(); ++itr) {
    delete (*itr).second;
  }
  inst_storage.clear();
  last_block_id = -1;


  /////////////////////////////////////////////////////////////////////////////////////////////
  // Calculate the resource occupancy of a new kernel
  /////////////////////////////////////////////////////////////////////////////////////////////
  int max_block = calculate_occupancy(kernel);




  // set kernel name
  char temp[10];
  kernel_name = kernel.name;

  // if same kernel has been called several times, corresponding suffix will be added
  // to the name of the kernel.
  if (kernel_count_map.find(kernel_name) != kernel_count_map.end()) {
    kernel_count = ++kernel_count_map[kernel_name];
  }
  else {
    kernel_count_map[kernel_name] = 0;
    kernel_count = 0;
  }

  sprintf(temp, "_%d", kernel_count);
  kernel_name.append(temp);
  kernel_name.append("/");

  string command("mkdir -p ");
  command.append(trace_path);
  command.append(kernel_name);

  string kernel_path = trace_path;
  kernel_path.append(kernel_name);


  /* to satisfy compiler */
  int status;
  status = system(command.c_str());
  if (status == -1) {
    status = system(command.c_str());
  }
  report("" << command.c_str() << " (status " <<  status << ")");


  string kernel_config_file = trace_path;
  kernel_config_file.append("kernel_config.txt");
  txt_kernel_config_file.open(kernel_config_file.c_str(), std::ios_base::app);

  // open kernel configuration file
  if (init == false) {
    txt_kernel_config_file << -1 << " newptx" << "\n";
  }

  txt_kernel_config_file << kernel_path << "Trace.txt\n";
  txt_kernel_config_file.close();


  char file_path[400];

#if WRITE_DEBUG == 1
  sprintf(file_path, "%s%sdebug.txt", trace_path.c_str(), kernel_name.c_str());
  debug_stream = new ofstream(file_path);
#endif


  // write config file in zlib format
  // no of threads
  // thread no and start instruction for each thread
  sprintf(file_path, "%s%s%s.%s", 
      trace_path.c_str(), kernel_name.c_str(), trace_name.c_str(), extension.c_str());
  gz_config_file = gzopen(file_path, "wb");

  // write number of threads
  if (sizeof(uint32_t) != gzwrite(gz_config_file, &num_total_warps, sizeof(uint32_t))) {
    std::cerr << "unable to write to config file\n";
    exit(-1);
  }


  /////////////////////////////////////////////////////////////////////////////////////////////
  // Get memory argument information
  /////////////////////////////////////////////////////////////////////////////////////////////
  sprintf(file_path, "%s%sdata.txt", trace_path.c_str(), kernel_name.c_str());
  ofstream data_file(file_path);

  data_file << "[MemoryArgument]\n";
  executive::Device::MemoryAllocationVector MAV = kernel.device->getAllAllocations();//event
  for (auto I = MAV.begin(), E = MAV.end(); I != E; ++I) {
    executive::Device::MemoryAllocation* ma = (*I);
    data_file << ma->pointer() << " " << ma->size() << "\n";;
//    report(ma->pointer() << " " << ma->size());
  }
  data_file.close();


  // write config file in text format : no of threads
  // thread no and start instruction for each thread
  sprintf(file_path, "%s%s%s.txt", trace_path.c_str(), kernel_name.c_str(), trace_name.c_str());
  txt_config_file.open(file_path);

  if (txt_config_file.fail())
    assert(0);

  txt_config_file << num_total_warps << " newptx";
  txt_config_file << " " << max_block << "\n";

  sprintf(file_path, "%s%s%s_info.txt", trace_path.c_str(), kernel_name.c_str(), trace_name.c_str());
  info_file.open(file_path);

  if (thread_info != NULL) {
    delete[] thread_info;
    delete[] trace_info;
  }

  thread_info = new Thread_info[num_total_warps];
  trace_info  = new Trace_info [num_total_warps];

  memset(thread_info, 0, num_total_warps * sizeof(Thread_info));
  memset(trace_info, 0, num_total_warps * sizeof(Trace_info));

  init = true;
}


///////////////////////////////////////////////////////////////////////////////////////////////
/// Called whenever an event takes place.
/// Note, the const reference 'event' is only valid until event() returns
///////////////////////////////////////////////////////////////////////////////////////////////
void trace::X86TraceGenerator::event(const trace::TraceEvent & event)
{
  ++total_inst_count;
  if (total_inst_count % 1000000 == 0)
    report(">>>>> inst_count:" << total_inst_count);
  // current block id
  int cur_block = event.blockId.y * gridDimX + event.blockId.x;


  ///
  /// Since each block is executed in sequential, we open gzFile for current block and
  /// close when it has been terminate. However, since we don't know when exactly a block
  /// terminates, we maintain last_block_id and compare with current block id.
  /// When ids do not match, a new block has been started.
  ///
  // open files on demand
  if (last_block_id != cur_block) {
    if (last_block_id != -1) {
      int block_id = last_block_id;
      // process each warps in previous block
      for (int ii = 0; ii < num_warps_per_block; ++ii) {
        // write remaining inst info
        Trace_info *tr_info = &trace_info[block_id * num_warps_per_block + ii];
        if (tr_info->bytes_accumulated) {
          int bytes_written = gzwrite(tr_info->trace_stream, tr_info->trace_buf, tr_info->bytes_accumulated); 
          if (bytes_written != tr_info->bytes_accumulated) {
            report("gz file write error");
            assert(0);
          }
        }
        gzclose(tr_info->trace_stream);

        // record warp information
        Thread_info *th_info = &thread_info[block_id * num_warps_per_block + ii];
        if (sizeof(Thread_info) != gzwrite(gz_config_file, th_info, sizeof(Thread_info))) {
          report("unable to write to config file");
          exit(-1);
        }

        txt_config_file << th_info->thread_id << " " << th_info->inst_count << "\n";
        info_file << th_info->thread_id << " " << th_info->inst_count << "\n";
      }
    }

    last_block_id = cur_block;


    // setup data structure for a new block
    for (int ii = 0; ii < num_warps_per_block; ++ii) {
      Thread_info *th_info = &thread_info[cur_block * num_warps_per_block + ii];
      Trace_info *tr_info  = &trace_info[cur_block * num_warps_per_block + ii];

      /* id assigned to each warp/block - shift block_id left by 16 and add the running thread count */
      int thread_id = (cur_block << 16) + ii;
      th_info->thread_id = thread_id;

      /* open trace file for each warp/thread */
      sprintf(tr_info->trace_name, "%s%s%s_%u.%s", 
          trace_path.c_str(), kernel_name.c_str(), trace_name.c_str(), th_info->thread_id, extension.c_str());

      tr_info->trace_stream = gzopen(tr_info->trace_name, "wb");
      if (tr_info->trace_stream == NULL) {
        report("unable to open file " << tr_info->trace_name);
        dbg << "unable to open file " << tr_info->trace_name
          << ", errorno is " << errno 
          << " message is " << strerror(errno) << "\n";
        assert(0);
      }
    }
  }


#if WRITE_DEBUG == 1
#if SINGLE_WARP == 1
  static int saved_cur_block = -1;
  static int saved_cur_warp = -1;

  if (saved_cur_block == -1) {
    saved_cur_block = cur_block;
  }
#endif
#endif



  /////////////////////////////////////////////////////////////////////////////////////////////
  // process current instruction
  /////////////////////////////////////////////////////////////////////////////////////////////

  const ir::PTXInstruction *ptx_inst = event.instruction;


  // Membar : memory barrier
  // Pmevent : performance monitor event
  // Currently, not supported
  if (ptx_inst->opcode == ir::PTXInstruction::Membar || 
      ptx_inst->opcode == ir::PTXInstruction::Pmevent) {
    report("opcode " << ir::PTXInstruction::toString(ptx_inst->opcode) << " seen");
    assert(0);
  }


  ///
  /// We maintain instruction hash per static instruction(key : PC address).
  /// By storing this information, we can reduce parsing (decoding) overhead
  ///

  // search current instruction in the instruction hash
  bool inst_found = false;
  Inst_info *inst_info;
  if (inst_storage.find(event.PC) != inst_storage.end()) {
    inst_info = inst_storage[event.PC];
    inst_found = true;
  }
  else {
    // add a new static instruction
    inst_info = new Inst_info;
    if (NULL == inst_info) {
      report("failed to allocated memory");
      exit(-1);
    }
    memset(inst_info, 0, sizeof(Inst_info));
    inst_storage.insert(pair<ir::PTXU64, Inst_info *>(event.PC, inst_info));
  }


  // seeing first instance of PC
  if (false == inst_found) {
    uint32_t src_count;
    uint32_t dest_count;


    // set sources and destinations

    /* source 1 */
    src_count = 0;
    if (ptx_inst->a.addressMode == ir::PTXOperand::Register || 
        ptx_inst->a.addressMode == ir::PTXOperand::Indirect) { 
      /* scalar */
      if (ptx_inst->a.vec == ir::PTXOperand::v1) {
        inst_info->src[src_count] = ptx_inst->a.reg;
        src_count++;
      }
      /* vector2, vector 4 */
      else if (ptx_inst->a.vec == ir::PTXOperand::v2 || 
          ptx_inst->a.vec ==  ir::PTXOperand::v4) {
        for (unsigned int i = 0; i < ptx_inst->a.array.size(); ++i) {
          inst_info->src[src_count] = ptx_inst->a.array[i].reg;
          src_count++;
        }
      }
      else
        assert(0);
    }

    /* source 2 */
    if (ptx_inst->b.addressMode == ir::PTXOperand::Register || 
        ptx_inst->b.addressMode == ir::PTXOperand::Indirect) { 
      if (ptx_inst->b.vec == ir::PTXOperand::v1) {
        inst_info->src[src_count] = ptx_inst->b.reg;
        src_count++;
      }
      else if (ptx_inst->b.vec == ir::PTXOperand::v2 || 
          ptx_inst->b.vec ==  ir::PTXOperand::v4) {
        for (unsigned int i = 0; i < ptx_inst->b.array.size(); ++i) {
          inst_info->src[src_count] = ptx_inst->b.array[i].reg;
          src_count++;
        }
      }
    }

    /* source 3 */
    if (ptx_inst->c.addressMode == ir::PTXOperand::Register || 
        ptx_inst->c.addressMode == ir::PTXOperand::Indirect) { 
      if (ptx_inst->c.vec ==  ir::PTXOperand::v1) {
        inst_info->src[src_count] = ptx_inst->c.reg;
        src_count++;
      }
      else if (ptx_inst->c.vec ==  ir::PTXOperand::v2 || 
          ptx_inst->c.vec ==  ir::PTXOperand::v4) {
        for (unsigned int i = 0; i < ptx_inst->c.array.size(); ++i) {
          inst_info->src[src_count] = ptx_inst->c.array[i].reg;
          src_count++;
        }
      }
    }


    // if predication variable is used, how do you specify that dependency? 
    // only predication registers will be used
    if (ptx_inst->pg.addressMode == ir::PTXOperand::Register && 
        ptx_inst->pg.type == ir::PTXOperand::pred && 
        ((ptx_inst->pg.condition == ir::PTXOperand::Pred) || 
         (ptx_inst->pg.condition == ir::PTXOperand::InvPred))) { 
      inst_info->src[src_count] = ptx_inst->pg.reg;
      src_count++;
    }


    /* destination register */
    dest_count = 0;
    if (ptx_inst->d.addressMode == ir::PTXOperand::Register || 
        ptx_inst->d.addressMode == ir::PTXOperand::Indirect) {
      if (ptx_inst->opcode != ir::PTXInstruction::St) {
        inst_info->dst[dest_count] = ptx_inst->d.reg;
        dest_count++;
      }
      else {
        if (ptx_inst->d.vec ==  ir::PTXOperand::v1) {
          inst_info->dst[dest_count] = ptx_inst->d.reg;
          dest_count++;
        }
        else if (ptx_inst->d.vec ==  ir::PTXOperand::v2 || 
            ptx_inst->d.vec ==  ir::PTXOperand::v4) {
          for (unsigned int i = 0; i < ptx_inst->d.array.size(); ++i) {
            inst_info->dst[dest_count] = ptx_inst->d.array[i].reg;
            dest_count++;
          }
        }
      }
    }

    /* second destination register for SetP */
    if (ptx_inst->pq.addressMode == ir::PTXOperand::Register || 
        ptx_inst->pq.addressMode == ir::PTXOperand::Indirect) {
      inst_info->dst[dest_count] = ptx_inst->pq.reg;
      dest_count++;
    }

    inst_info->num_read_regs = src_count;
    inst_info->num_dest_regs = dest_count;

    assert(src_count <= MAX_SRC_NUM && dest_count <= MAX_DST_NUM);


    // set has_immediate flag - this flag is used to indicate that 
    // successive memory accesses found in the trace are from the 
    // same instruction (uncoaleced memory accesses)
    inst_info->has_immediate = false;

    ///////////////////////////////////////////////////////////////////////////////////////////
    // Opcode parsing begin
    ///////////////////////////////////////////////////////////////////////////////////////////
    
    // load
    // TOCHECK (generic type not in ptx document)
    if (ptx_inst->opcode == ir::PTXInstruction::Ld ||
        ptx_inst->opcode == ir::PTXInstruction::Ldu) {
      switch (ptx_inst->addressSpace) {
        case ir::PTXInstruction::Const:
          inst_info->opcode = TR_MEM_LD_CM;
          break;

        case ir::PTXInstruction::Global:
        case ir::PTXInstruction::Generic:
          inst_info->opcode = TR_MEM_LD_GM; 
          break;

        case ir::PTXInstruction::Local:
          inst_info->opcode = TR_MEM_LD_LM; 
          break;

        case ir::PTXInstruction::Param:
          inst_info->opcode = TR_MEM_LD_PM; 
          break;

        case ir::PTXInstruction::Shared:
          inst_info->opcode = TR_MEM_LD_SM; 
          break;

        case ir::PTXInstruction::Texture:
          report("texture load - tocheck");
          assert(0);
          break;

        default:
          assert(0);
      }
    }
    // FIXME : support LDU
    // load uniform
    else if (0 && ptx_inst->opcode == ir::PTXInstruction::Ldu) {
      report("LDU currently not supported");
      assert(0);
      switch (ptx_inst->addressSpace) {
        case ir::PTXInstruction::Global:
          inst_info->opcode = LDU_GM;
          break;

        default:
          assert(0);
      }
    }
    // store 
    else if (ptx_inst->opcode == ir::PTXInstruction::St) {
      switch (ptx_inst->addressSpace) {
        case ir::PTXInstruction::Global:
        case ir::PTXInstruction::Generic:
          inst_info->opcode = TR_MEM_ST_GM;
          break;

        case ir::PTXInstruction::Local:
          inst_info->opcode = TR_MEM_ST_LM;
          break;

        case ir::PTXInstruction::Shared:
          inst_info->opcode = TR_MEM_ST_SM;
          break;

        default:
          assert(0);
      }
    }
    // prefetch instruction
    // TODO (jaekyu, 8-5-2010)
    else if (ptx_inst->opcode == ir::PTXInstruction::Prefetch) {
      report("PREF currently not supported");
      assert(0);
      /*
      switch (ptx_inst->addressSpace) {
        case ir::PTXInstruction::Global:
          inst_info->opcode = PREF_GM_L1;
          break;

        case ir::PTXInstruction::Local:
          inst_info->opcode = PREF_LM_L1;
          break;

        default:
          assert(0);
      }
      */
    }
    // prefetch uniform instruction
    // TODO (jaekyu, 8-5-2010)
    else if (ptx_inst->opcode == ir::PTXInstruction::Prefetchu) {
      report("PREF_UNIFORM currently not supported");
      assert(0);
      /*
      inst_info->opcode = PREF_UNIFORM;
      */
    }
    // texture memory
    else if (TEX_INST(ptx_inst->opcode)) {
      inst_info->opcode = TR_MEM_LD_TM;
    }
    // surface memory access
    // TODO (jaekyu, 8-5-2010)
    else if (SUR_INST(ptx_inst->opcode)) {
      report("Surface memory is not supported. 8-3-2010 Jaekyu Lee");
      assert(0);
    }
    // Atomic & Reduction
    // .global, .shared
    else if (ptx_inst->opcode == ir::PTXInstruction::Atom || 
             ptx_inst->opcode == ir::PTXInstruction::Red) {
      switch (ptx_inst->addressSpace) {
        case ir::PTXInstruction::Global:
          inst_info->opcode = TR_DATA_XFER_GM;
          break;

        case ir::PTXInstruction::Shared:
          inst_info->opcode = TR_DATA_XFER_SM;
          break;

        default :
          assert(0);
          break;
      }
    }
    // other instructions
    else {
#ifndef GPU_VALIDATION
      // floating point
      if (ptx_inst->type == ir::PTXOperand::f16 || 
          ptx_inst->type == ir::PTXOperand::f32 || 
          ptx_inst->type == ir::PTXOperand::f64) {
        inst_info->opcode = ptx_to_x86[1][ptx_inst->opcode];
        inst_info->is_fp = true;
      }
      // integer
      else {
        inst_info->opcode = ptx_to_x86[0][ptx_inst->opcode];
        inst_info->is_fp = false;
      }
#else   
      // double
      if (ptx_inst->type == ir::PTXOperand::f64) {
        inst_info->opcode = ptx_to_x86[3][ptx_inst->opcode];
        inst_info->is_fp = true;
      }
      // floating point
      else if (ptx_inst->type == ir::PTXOperand::f16 || 
          ptx_inst->type == ir::PTXOperand::f32) {
        inst_info->opcode = ptx_to_x86[2][ptx_inst->opcode];
        inst_info->is_fp = true;
      }
      // integer - 64-bit
      else if (ptx_inst->type == ir::PTXOperand::s64 ||
          ptx_inst->type == ir::PTXOperand::u64 ||
          ptx_inst->type == ir::PTXOperand::b64) {
        inst_info->opcode = ptx_to_x86[1][ptx_inst->opcode];
        inst_info->is_fp = false;
      }
      // integer
      else {
        inst_info->opcode = ptx_to_x86[0][ptx_inst->opcode];
        inst_info->is_fp = false;
      }
#endif
    }
    ///////////////////////////////////////////////////////////////////////////////////////////
    // Opcode parsing done
    ///////////////////////////////////////////////////////////////////////////////////////////


#ifdef GPU_VALIDATION
    if (inst_info->opcode == XED_CATEGORY_INVALID) {
      report("invalid opcode for " << ptx_inst->opcode);
      cout << "ptx inst is " << ir::PTXInstruction::toString(ptx_inst->opcode) << " inst type is " << ptx_inst->type << "\n";
      assert(0);
    }
#endif

    // set cf_type
    switch (ptx_inst->opcode) {
      case ir::PTXInstruction::Bra:
        inst_info->cf_type = CF_CBR;
        break;

      case ir::PTXInstruction::Call:
        inst_info->cf_type = CF_CALL;
        break;

      case ir::PTXInstruction::Exit:
        inst_info->cf_type = CF_SYS;
        break;

      case ir::PTXInstruction::Ret:
        inst_info->cf_type = CF_RET;
        break;

      default:
        inst_info->cf_type = NOT_CF;
        break;
    }


    // set has_st flag
    inst_info->has_st = ST_INST(ptx_inst->opcode);

    // set write_flg flag - this flag is not used by macsim, so setting it to false
    inst_info->write_flg = false;

    // set num_ld
    if (ptx_inst->opcode == ir::PTXInstruction::Ld || 
        ptx_inst->opcode == ir::PTXInstruction::Ldu ||
        ptx_inst->opcode == ir::PTXInstruction::Prefetch ||
        ptx_inst->opcode == ir::PTXInstruction::Prefetchu ||
        ptx_inst->opcode == ir::PTXInstruction::Atom ||
        ptx_inst->opcode == ir::PTXInstruction::Red ||
        TEX_INST(ptx_inst->opcode)) {
      inst_info->num_ld = 1;
    }
    else {
      inst_info->num_ld = 0;
    }

    /* set instruction size */
    inst_info->size = 4;
  } // A newly seen instruction has been decoded


  /////////////////////////////////////////////////////////////////////////////////////////////
  // current instruction has been decoded at this point
  /////////////////////////////////////////////////////////////////////////////////////////////
  int cur_warp;
  boost::dynamic_bitset<>::size_type pos;
  int warp_set_bit_count;
  bool branch_taken;
  Trace_info *info;
  unsigned int warp_active_mask;
  unsigned int branch_taken_mask;
  unsigned int mem_count;

  cur_warp = 0;
  pos = event.active.find_first();
  warp_set_bit_count = 0;
  memset(mem_access, 0, sizeof(MemAccess) * WARP_SIZE);


  // flag to indicate whether inst. is a control flow inst. or not 
  bool branch_inst = BRANCH_INST(ptx_inst->opcode);

  // flag to indicate whether inst. accesses memory or not 
  bool mem_inst = false;
  if (inst_info->num_ld >= 1 || inst_info->has_st == true) {
    mem_inst = true;
  }


  if (mem_inst && event.memory_addresses.size() == 0) {
    report("ERROR: No memory addresses found opcode:" << ir::PTXInstruction::toString(ptx_inst->opcode));
    assert(0);
    inst_info->num_ld = 0;
    inst_info->has_st = false;
    mem_inst = false;
  }


  // assumption : there are no mem-to-mem operations (atom, red may be mem-to-mem operations 
  // but the memory address list has only one address value for these instructions - expl better
  mem_count = 0;
  do {
    warp_set_bit_count = 0;
    branch_taken       = false;
    warp_active_mask   = 0;
    branch_taken_mask  = 0;

    // for current warp check if any thread was active
    // if threads were active collect active mask, memory addresses accessed (if any) and branch taken mask (if applicable)
    // if all threads were inactive check the next warp for active threads
    while (pos != boost::dynamic_bitset<>::npos && pos < (unsigned int)((cur_warp + 1) * WARP_SIZE)) {
      if (mem_inst) {
        if (mem_count >= event.memory_addresses.size()) {
          cout << "mem_count:" << mem_count << " " << event.memory_addresses.size() << "\n";
          cerr << "opcode " << ir::PTXInstruction::toString(ptx_inst->opcode) << "\n";
          assert(mem_count < event.memory_addresses.size());
        }
        mem_access[warp_set_bit_count].mem_addr = event.memory_addresses[mem_count] + INST_LAST_ADDR;
        mem_access[warp_set_bit_count].mem_size = event.memory_size;
        mem_count++;
      }
      else if (branch_inst) {
        if (pos < event.taken.size() && event.taken.test(pos)) {
          branch_taken_mask |= (1 << (pos % WARP_SIZE));
        }

        if (!branch_taken) {
          // check only for bra instruction, what about other instructions? 
          // always taken - but what if they are predicated???
          if (ptx_inst->opcode == ir::PTXInstruction::Bra) { 
            if (event.taken.test(pos)) {
              branch_taken = true;
            }
          }
          else {
            branch_taken = true;
          }
        }
      }

      warp_active_mask |= (1 << (pos % WARP_SIZE));
      warp_set_bit_count++;
      pos = event.active.find_next(pos);
    }


    // if current warp had active threads then generate trace for the executed instruction
    if (warp_set_bit_count) {
      warp_active_mask = ~warp_active_mask;
      branch_taken_mask = ~branch_taken_mask;

      // store active thread mask in ld_vaddr2 */
      inst_info->ld_vaddr2 = warp_active_mask;

      // ocelot generates PC index instead of PC address, since we are assuming 
      // that instruction size is 4 bytes, left-shift PC index by 2
      inst_info->instruction_addr = (event.PC << 2) + INST_START_ADDR;

      if ((inst_info->instruction_addr + 3) > INST_LAST_ADDR) {
        report("[assert] inst_info->instruction_addr + 3 > INST_LAST_ADDR");
        report("pc:" << hex << event.PC << " " << inst_info->instruction_addr);
        report("start:" << hex << INST_START_ADDR << " " << INST_LAST_ADDR);
        assert((inst_info->instruction_addr + 3)<= INST_LAST_ADDR);
      }


      inst_info->branch_target = 0; 
      inst_info->actually_taken = 0; 
      inst_info->st_vaddr = inst_info->mem_write_size = 0;
      inst_info->ld_vaddr1 = inst_info->mem_read_size = 0;
      inst_info->rep_dir = 0;


      // for instructions that access memory coalesce requests into fewer, 
      // larger contiguous chunks of requests if possible MACSIM will then 
      // break these larger requests into cache block size requests
      if (mem_inst) {
        MemAccess temp;

        // sort addresses based on address and memory access size
        for (int i = 0; i < (warp_set_bit_count - 1); ++i) {
          for (int j = 0; j < (warp_set_bit_count - i - 1); j++) {
            if ((mem_access[j].mem_addr > mem_access[j + 1].mem_addr) 
                || ((mem_access[j].mem_addr == mem_access[j + 1].mem_addr) 
                  && (mem_access[j].mem_size > mem_access[j + 1].mem_size))) {
              temp = mem_access[j];
              mem_access[j] = mem_access[j + 1];
              mem_access[j + 1] = temp;
            }
          }
        }

        // merge overlapping accesses 
        // if [i] and [i + 1] are overlapping, merge [i] into [i + 1]
        for (int i = 0 ; i < (warp_set_bit_count - 1); ++i) {
          if (mem_access[i].mem_addr == mem_access[i + 1].mem_addr) {
            mem_access[i].mem_size = 0;
          }
          else if ((mem_access[i].mem_addr + mem_access[i].mem_size) == 
              mem_access[i + 1].mem_addr) {
            mem_access[i + 1].mem_addr = mem_access[i].mem_addr;
            mem_access[i + 1].mem_size += mem_access[i].mem_size;

            mem_access[i].mem_size = 0;
          }
          else if ((mem_access[i].mem_addr + mem_access[i].mem_size) > mem_access[i + 1].mem_addr
              && (mem_access[i].mem_addr + mem_access[i].mem_size) <= (mem_access[i + 1].mem_addr + mem_access[i + 1].mem_size)) {
            mem_access[i + 1].mem_size = (mem_access[i + 1].mem_addr + mem_access[i + 1].mem_size) - mem_access[i].mem_addr;
            mem_access[i + 1].mem_addr = mem_access[i].mem_addr;

            mem_access[i].mem_size = 0;
          }
          else if ((mem_access[i].mem_addr + mem_access[i].mem_size) > mem_access[i + 1].mem_addr
              && (mem_access[i].mem_addr + mem_access[i].mem_size) > (mem_access[i + 1].mem_addr + mem_access[i + 1].mem_size)) {
            mem_access[i + 1].mem_addr = mem_access[i].mem_addr;
            mem_access[i + 1].mem_size = mem_access[i].mem_size;

            mem_access[i].mem_size = 0;
          }
        }

        // move merged accesses to beginning of array
        // if prev step was done in reverse, then we could have skipped this step*/ 
        int count = 0;
        for (int i = 0; i < warp_set_bit_count; ++i) {
          if (mem_access[i].mem_size) {
            mem_access[count].mem_addr = mem_access[i].mem_addr;
            mem_access[count].mem_size = mem_access[i].mem_size;
            count++;
          }
        }


        // generate instructions
        for (int i = 0; i < count; ++i) {
          /* MACSIM uses only 8-bit fields for memory address sizes */
          assert(mem_access[i].mem_size < 256);

          if (ptx_inst->opcode == ir::PTXInstruction::Ld || 
              ptx_inst->opcode == ir::PTXInstruction::Mov || 
              ptx_inst->opcode == ir::PTXInstruction::Ldu ||
              ptx_inst->opcode == ir::PTXInstruction::Prefetch ||
              ptx_inst->opcode == ir::PTXInstruction::Prefetchu ||
              inst_info->opcode == TR_MEM_LD_TM ||
              ptx_inst->opcode == ir::PTXInstruction::Atom || 
              ptx_inst->opcode == ir::PTXInstruction::Red) {
            // set ld_vaddr1 and mem_read_size
            inst_info->ld_vaddr1 = (mem_access[i].mem_addr & mem_addr_flag);
            inst_info->mem_read_size = mem_access[i].mem_size;
          }


          if (ptx_inst->opcode == ir::PTXInstruction::St || 
              ptx_inst->opcode == ir::PTXInstruction::Atom ||
              ptx_inst->opcode == ir::PTXInstruction::Red) {
            // set st_vaddr and mem_write_size
            inst_info->st_vaddr = (mem_access[i].mem_addr & mem_addr_flag);
            inst_info->mem_write_size = mem_access[i].mem_size;
          }

#if WRITE_DEBUG == 1
#if SINGLE_WARP == 1
          if (saved_cur_warp == -1) {
            saved_cur_warp = cur_warp;
          }


          if (saved_cur_block == cur_block && saved_cur_warp == cur_warp) {
#endif
            *debug_stream << "opcode " << ir::PTXInstruction::toString(ptx_inst->opcode) 
              << " a " << ptx_inst->a.identifier << " b " << ptx_inst->b.identifier 
              << " c " << ptx_inst->c.identifier << " d " << ptx_inst->d.identifier << "\n";
            writeInstToFile(*debug_stream, inst_info);
#if 0
          }
#endif
#if SINGLE_WARP == 1
        }
#endif
#endif
        assert((cur_block * num_warps_per_block + cur_warp) < num_total_warps);

        info = &trace_info[cur_block * num_warps_per_block + cur_warp];

        if ((i == 0 || i == (count - 1)) && count > 1) {
          // macsim does not use this field, so using it to indicate
          // whether instruction is first/last instruction in the 
          // sequence of instructions generated for an uncoalesced 
          // instruction
          inst_info->has_immediate = true; 
        }
        else {
          //macsim does not use this field, so using it to indicate
          //whether instruction is first/last instruction in the 
          //sequence of instructions generated for an uncoalesced 
          //instruction
          inst_info->has_immediate = false;
        }

        memcpy(info->trace_buf + info->bytes_accumulated, inst_info, sizeof(Inst_info));
        info->bytes_accumulated += sizeof(Inst_info);
        if (info->bytes_accumulated == BUF_SIZE) {
          int bytes_written;
          bytes_written = gzwrite(info->trace_stream, info->trace_buf, BUF_SIZE);
          if (bytes_written != BUF_SIZE) {
            std::cerr << "unable to write to trace file, thread " << (cur_block * num_warps_per_block + cur_warp) 
              << " bytes written = " <<  bytes_written << "\n";
            int err;
            std::cerr << "1. error is " << gzerror(info->trace_stream, &err) << "\n";
            if (err != Z_ERRNO) {
              std::cerr << "error code is " << err << "\n";
            }
            else {
              std::cerr << "error is " << strerror(errno) << "\n";
            }

            exit(-1);
          }

          info->bytes_accumulated = 0;
        }
        info->inst_count++;
      }
      memset(mem_access, 0, sizeof(MemAccess) * WARP_SIZE);
    }
    else if (branch_inst) {
      if (ptx_inst->opcode == ir::PTXInstruction::Bra || 
          ptx_inst->opcode == ir::PTXInstruction::Call ||
          ptx_inst->opcode == ir::PTXInstruction::Ret) {
        inst_info->branch_target = (ptx_inst->branchTargetInstruction << 2) + INST_START_ADDR; //check this

        inst_info->ld_vaddr1 = branch_taken_mask;
        inst_info->st_vaddr = (ptx_inst->reconvergeInstruction << 2) + INST_START_ADDR;
      }
      inst_info->actually_taken = branch_taken;


#if WRITE_DEBUG == 1
#if SINGLE_WARP == 1
      if (saved_cur_warp == -1) {
        saved_cur_warp = cur_warp;
      }


      if (saved_cur_block == cur_block && saved_cur_warp == cur_warp) {
#endif
        *debug_stream << "opcode " << ir::PTXInstruction::toString(ptx_inst->opcode) 
          << " a " << ptx_inst->a.identifier << " b " << ptx_inst->b.identifier 
          << " c " << ptx_inst->c.identifier << " d " << ptx_inst->d.identifier << "\n";
        writeInstToFile(*debug_stream, inst_info);
#if SINGLE_WARP == 1
      }
#endif
#endif
      assert((cur_block * num_warps_per_block + cur_warp) < num_total_warps);
      info = &trace_info[cur_block * num_warps_per_block + cur_warp];
      memcpy(info->trace_buf + info->bytes_accumulated, inst_info, sizeof(Inst_info));
      info->bytes_accumulated += sizeof(Inst_info);
      if (info->bytes_accumulated == BUF_SIZE) {
        int bytes_written;
        bytes_written = gzwrite(info->trace_stream, info->trace_buf, BUF_SIZE);
        if (bytes_written != BUF_SIZE) {
          std::cerr << "unable to write to trace file, thread " << (cur_block * num_warps_per_block + cur_warp) 
            << " bytes written = " <<  bytes_written << "\n";
          int err;
          std::cerr << "2. error is " << gzerror(info->trace_stream, &err) << "\n";
          if (err != Z_ERRNO) {
            std::cerr << "error code is " << err << "\n";
          }
          else {
            std::cerr << "error is " << strerror(errno) << "\n";
          }

          exit(-1);
        }

        info->bytes_accumulated = 0;
      }
      info->inst_count++;
    }
    else {
#if WRITE_DEBUG == 1
#if SINGLE_WARP == 1
      if (saved_cur_warp == -1) {
        saved_cur_warp = cur_warp;
      }


      if (saved_cur_block == cur_block && saved_cur_warp == cur_warp) {
#endif
        *debug_stream << "opcode " << ir::PTXInstruction::toString(ptx_inst->opcode) 
          << " a " << ptx_inst->a.identifier << " b " << ptx_inst->b.identifier 
          << " c " << ptx_inst->c.identifier << " d " << ptx_inst->d.identifier << "\n";
        writeInstToFile(*debug_stream, inst_info);
#if SINGLE_WARP == 1
      }
#endif
#endif


      assert((cur_block * num_warps_per_block + cur_warp) < num_total_warps);

      info = &trace_info[cur_block * num_warps_per_block + cur_warp];
      memcpy(info->trace_buf + info->bytes_accumulated, inst_info, sizeof(Inst_info));
      info->bytes_accumulated += sizeof(Inst_info);
      if (info->bytes_accumulated == BUF_SIZE) {
        int bytes_written;
        bytes_written = gzwrite(info->trace_stream, info->trace_buf, BUF_SIZE);
        if (bytes_written != BUF_SIZE) {
          std::cerr 
            << "unable to write to trace file, thread " << (cur_block * num_warps_per_block + cur_warp) 
            << " block " << cur_block
            << " bytes written = " <<  bytes_written  
            << " file_name:" << info->trace_name << "\n";
          int err;
          std::cerr << "3. error is " << gzerror(info->trace_stream, &err) << "\n";
          if (err != Z_ERRNO) {
            std::cerr << "error code is " << err << "\n";
          }
          else {
            std::cerr << "error is " << strerror(errno) << "\n";
          }

          exit(-1);
        }

        info->bytes_accumulated = 0;
      }
      info->inst_count++;
    }
  }
  cur_warp++;

} while (pos != boost::dynamic_bitset<>::npos);
}


///////////////////////////////////////////////////////////////////////////////////////////////
// finalize
///////////////////////////////////////////////////////////////////////////////////////////////
void trace::X86TraceGenerator::finalize()
{
  report("finalize");
  if (!init)
    return ;

  report("total instruction count: " << total_inst_count);


  int block_id = last_block_id;
  for (int ii = 0; ii < num_warps_per_block; ++ii) {
    Trace_info *tr_info = &trace_info[block_id * num_warps_per_block + ii];
    if (tr_info->bytes_accumulated) {
      int bytes_written = gzwrite(tr_info->trace_stream, tr_info->trace_buf, tr_info->bytes_accumulated); 
      if (bytes_written != tr_info->bytes_accumulated) {
        std::cerr << "gz file write error\n";
        assert(0);
      }
    }
    gzclose(tr_info->trace_stream);

    Thread_info *th_info = &thread_info[block_id * num_warps_per_block + ii];
    if (sizeof(Thread_info) != gzwrite(gz_config_file, th_info, sizeof(Thread_info))) {
      std::cerr << "unable to write to config file\n";
      exit(-1);
    }

    txt_config_file << th_info->thread_id << " " << th_info->inst_count << "\n";
    info_file << th_info->thread_id << " " << th_info->inst_count << "\n";
  }


  // close config and info files
  gzclose(gz_config_file);
  txt_config_file.close();
  info_file.close();

}


///////////////////////////////////////////////////////////////////////////////////////////////
// for debug purpose, write instruction info in a file (text format)
///////////////////////////////////////////////////////////////////////////////////////////////
void trace::X86TraceGenerator::writeInstToFile(ofstream &file, Inst_info *t_info)
{
  file << "*** begin of the data strcture *** " <<endl;

  file << "t_info->uop_opcode " << dec << t_info->opcode << endl;
  file << "t_info->num_read_regs: " << hex <<  (uint32_t) t_info->num_read_regs << endl;
  file << "t_info->num_dest_regs: " << hex << (uint32_t) t_info->num_dest_regs << endl;

  file << "t_info->src1: " << hex << (uint32_t) t_info->src[0] << endl;
  file << "t_info->src2: " << hex << (uint32_t) t_info->src[1] << endl;
  file << "t_info->src3: " << hex << (uint32_t) t_info->src[2] << endl;
  file << "t_info->src4: " << hex << (uint32_t) t_info->src[3] << endl;


  file << "t_info->dest1: " << hex << (uint32_t) t_info->dst[0] << endl;
  file << "t_info->dest2: " << hex << (uint32_t) t_info->dst[1] << endl;
  file << "t_info->dest3: " << hex << (uint32_t) t_info->dst[2] << endl;
  file << "t_info->dest4: " << hex << (uint32_t) t_info->dst[3] << endl;

  file << "t_info->cf_type: " << hex << (uint32_t) t_info->cf_type << endl;
  file << "t_info->has_immediate: " << hex << (uint32_t) t_info->has_immediate << endl;


  file << "t_info->r_dir:" << (uint32_t) t_info->rep_dir << endl;
  file << "t_info->has_st: " << hex << (uint32_t) t_info->has_st << endl;
  file << "t_info->num_ld: " << hex << (uint32_t) t_info->num_ld << endl;
  file << "t_info->mem_read_size: " << hex << (uint32_t) t_info->mem_read_size << endl;
  file << "t_info->mem_write_size: " << hex << (uint32_t) t_info->mem_write_size << endl;
  file << "t_info->is_fp: " << (uint32_t) t_info->is_fp << endl;

  file << "t_info->ld_vaddr1: " << hex << (uint32_t) t_info->ld_vaddr1 << endl;
  file << "t_info->ld_vaddr2: " << hex << (uint32_t) t_info->ld_vaddr2 << endl;
  file << "t_info->st_vaddr: " << hex << (uint32_t) t_info->st_vaddr << endl;

  file << "t_info->instruction_addr: " << dec << (uint32_t) t_info->instruction_addr << endl;

  file << "t_info->branch_target: " << dec << (uint32_t) t_info->branch_target << endl;
  file << "t_info->actually_taken: " << hex << (uint32_t) t_info->actually_taken << endl;
  file << "t_info->write_flg: " << hex << (uint32_t) t_info->write_flg << endl;

  file << "*** end of the data strcture *** " << endl << endl;
}


