#ifndef DMASG_CONFIG_H
#define DMASG_CONFIG_H

//For DMA interrupt
uint32_t cam1_s2mm_active;
uint32_t cam2_s2mm_active;
uint32_t display_mm2s_active;
uint32_t hw_accel_s2mm_active;
uint32_t hw_accel_mm2s_1_active;
uint32_t hw_accel_mm2s_2_active;

#define DMASG_BASE            IO_APB_SLAVE_1_APB
#define PLIC_DMASG_CHANNEL    SYSTEM_PLIC_USER_INTERRUPT_A_INTERRUPT

//Each channel connects to only 1 port, hence all ports are referred as port 0.
#define DMASG_CAM1_S2MM_CHANNEL        0
#define DMASG_CAM1_S2MM_PORT           0

#define DMASG_CAM2_S2MM_CHANNEL        1
#define DMASG_CAM2_S2MM_PORT           0

#define DMASG_DISPLAY_MM2S_CHANNEL     2
#define DMASG_DISPLAY_MM2S_PORT        0

#define DMASG_HW_ACCEL_S2MM_CHANNEL    3
#define DMASG_HW_ACCEL_S2MM_PORT       0

#define DMASG_HW_ACCEL_MM2S_1_CHANNEL  4
#define DMASG_HW_ACCEL_MM2S_1_PORT     0

#define DMASG_HW_ACCEL_MM2S_2_CHANNEL  5
#define DMASG_HW_ACCEL_MM2S_2_PORT     0

void trap_entry();

void dma_init(){
   //configure PLIC
   plic_set_threshold(BSP_PLIC, BSP_PLIC_CPU_0, 0); //cpu 0 accept all interrupts with priority above 0
   
   //enable PLIC DMASG channel 0 interrupt listening (But for the demo, we enable the DMASG internal interrupts later)
   plic_set_enable(BSP_PLIC, BSP_PLIC_CPU_0, PLIC_DMASG_CHANNEL, 1);
   plic_set_priority(BSP_PLIC, PLIC_DMASG_CHANNEL, 1);
   
   //enable interrupts
   csr_write(mtvec, trap_entry); //Set the machine trap vector (../common/trap.S)
   csr_set(mie, MIE_MEIE); //Enable external interrupts
   csr_write(mstatus, MSTATUS_MPP | MSTATUS_MIE);
}

//Used on unexpected trap/interrupt codes
void crash(){
   bsp_putString("\n*** CRASH ***\n");
   while(1);
}

void externalInterrupt(){
   uint32_t claim;
   //While there is pending interrupts
   while(claim = plic_claim(BSP_PLIC, BSP_PLIC_CPU_0)){
      switch(claim){
      case PLIC_DMASG_CHANNEL:
         if(cam1_s2mm_active && !(dmasg_busy(DMASG_BASE, DMASG_CAM1_S2MM_CHANNEL))) {
            dmasg_interrupt_config(DMASG_BASE, DMASG_CAM1_S2MM_CHANNEL, 0);         //Disable dmasg channel interrupt
            cam1_s2mm_active = 0;
            //interrupt_service_routine();
         }
         if(cam2_s2mm_active && !(dmasg_busy(DMASG_BASE, DMASG_CAM2_S2MM_CHANNEL))) {
            dmasg_interrupt_config(DMASG_BASE, DMASG_CAM2_S2MM_CHANNEL, 0);         //Disable dmasg channel interrupt
            cam2_s2mm_active = 0;
            //interrupt_service_routine();
         }
         if(display_mm2s_active && !(dmasg_busy(DMASG_BASE, DMASG_DISPLAY_MM2S_CHANNEL))) {
            dmasg_interrupt_config(DMASG_BASE, DMASG_DISPLAY_MM2S_CHANNEL, 0);      //Disable dmasg channel interrupt
            display_mm2s_active = 0;
            //interrupt_service_routine();
         }
         if(hw_accel_s2mm_active && !(dmasg_busy(DMASG_BASE, DMASG_HW_ACCEL_S2MM_CHANNEL))) {
            dmasg_interrupt_config(DMASG_BASE, DMASG_HW_ACCEL_S2MM_CHANNEL, 0);     //Disable dmasg channel interrupt
            hw_accel_s2mm_active = 0;
            //interrupt_service_routine();
         }
         if(hw_accel_mm2s_1_active && !(dmasg_busy(DMASG_BASE, DMASG_HW_ACCEL_MM2S_1_CHANNEL))) {
            dmasg_interrupt_config(DMASG_BASE, DMASG_HW_ACCEL_MM2S_1_CHANNEL, 0);   //Disable dmasg channel interrupt
            hw_accel_mm2s_1_active = 0;
            //interrupt_service_routine();
         }
         if(hw_accel_mm2s_2_active && !(dmasg_busy(DMASG_BASE, DMASG_HW_ACCEL_MM2S_2_CHANNEL))) {
            dmasg_interrupt_config(DMASG_BASE, DMASG_HW_ACCEL_MM2S_2_CHANNEL, 0);   //Disable dmasg channel interrupt
            hw_accel_mm2s_2_active = 0;
            //interrupt_service_routine();
         }
         break;
      default: crash(); break;
      }
      plic_release(BSP_PLIC, BSP_PLIC_CPU_0, claim); //unmask the claimed interrupt
   }
}

//Called by trap_entry on both exceptions and interrupts events
void trap(){
   int32_t mcause = csr_read(mcause);
   int32_t interrupt = mcause < 0;    //Interrupt if true, exception if false
   int32_t cause     = mcause & 0xF;
   if(interrupt){
      switch(cause){
      case CAUSE_MACHINE_EXTERNAL: externalInterrupt(); break;
      default: crash(); break;
      }
   } else {
      crash();
   }
}

void flush_data_cache(){
   asm(".word(0x500F)");
}

#endif
