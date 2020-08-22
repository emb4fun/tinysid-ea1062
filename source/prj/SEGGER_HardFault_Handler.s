
/*
 * See https://wiki.segger.com/Cortex-M_Fault and application note AN00016:
 *    https://www.segger.com/downloads/application-notes/AN00016
 */ 

        .syntax unified

        .extern HardFaultHandler

        .global HardFault_Handler
        .type   HardFault_Handler, function

        .section .init, "ax"
        .thumb_func

 ;/*********************************************************************
 ;*
 ;*      HardFault_Handler()
 ;*
 ;*  Function description
 ;*    Evaluates the used stack (MSP, PSP) and passes the appropiate
 ;*    stack pointer to the HardFaultHandler "C"-routine.
 ;*
 ;*  Notes
 ;*    (1) Ensure that HardFault_Handler is part of the exception table
 ;*/

HardFault_Handler:
 #if (defined (__IAR_SYSTEMS_ASM__) && (__ARM6M__) && (__CORE__ == __ARM6M__)) || \
     (defined(__CC_ARM) || (defined __clang__)) && (__TARGET_ARCH_6S_M)        || \
     (defined (__GNUC__) && ((__ARM_ARCH_6M__) || (__ARM_ARCH_8M_BASE__)))
         ;// This version is for Cortex M0
         movs   R0, #4
         mov    R1, LR
         tst    R0, R1            ;// Check EXC_RETURN in Link register bit 2.
         bne    Uses_PSP
         mrs    R0, MSP           ;// Stacking was using MSP.
         b      Pass_StackPtr
 Uses_PSP:
         mrs    R0, PSP           ;// Stacking was using PSP.
 Pass_StackPtr:
         ldr    R2,=HardFaultHandler
         bx     R2                ;// Stack pointer passed through R0. 
 #else
         ;// This version is for Cortex M3, Cortex M4 and Cortex M4F
         tst    LR, #4            ;// Check EXC_RETURN in Link register bit 2.
         ite    EQ
         mrseq  R0, MSP           ;// Stacking was using MSP.
         mrsne  R0, PSP           ;// Stacking was using PSP.
         b      HardFaultHandler  ;// Stack pointer passed through R0.
 #endif
         .end
 
