#include <i686/interrupts/Interrupts.hpp>

#include <i686/interrupts/IDT.hpp>
#include <i686/interrupts/ISR.hpp>
#include <i686/internals/Segmentation.hpp>

#include <generic/Init.hpp>
#include <generic/tasks/Scheduler.hpp>

#include <librt/Panic.hpp>
#include <librt/Log.hpp>

#include <stddef.h>

namespace core::interrupts
{
  typedef void(*isr_t)();

  static constexpr size_t IDT_SIZE = 256;
  constexpr isr_t isrs[IDT_SIZE] = {
    &isr0,   &isr1,   &isr2,   &isr3,   &isr4,   &isr5,   &isr6,   &isr7,   &isr8,   &isr9,   &isr10,  &isr11,  &isr12,  &isr13,  &isr14,  &isr15,
    &isr16,  &isr17,  &isr18,  &isr19,  &isr20,  &isr21,  &isr22,  &isr23,  &isr24,  &isr25,  &isr26,  &isr27,  &isr28,  &isr29,  &isr30,  &isr31,
    &isr32,  &isr33,  &isr34,  &isr35,  &isr36,  &isr37,  &isr38,  &isr39,  &isr40,  &isr41,  &isr42,  &isr43,  &isr44,  &isr45,  &isr46,  &isr47,
    &isr48,  &isr49,  &isr50,  &isr51,  &isr52,  &isr53,  &isr54,  &isr55,  &isr56,  &isr57,  &isr58,  &isr59,  &isr60,  &isr61,  &isr62,  &isr63,
    &isr64,  &isr65,  &isr66,  &isr67,  &isr68,  &isr69,  &isr70,  &isr71,  &isr72,  &isr73,  &isr74,  &isr75,  &isr76,  &isr77,  &isr78,  &isr79,
    &isr80,  &isr81,  &isr82,  &isr83,  &isr84,  &isr85,  &isr86,  &isr87,  &isr88,  &isr89,  &isr90,  &isr91,  &isr92,  &isr93,  &isr94,  &isr95,
    &isr96,  &isr97,  &isr98,  &isr99,  &isr100, &isr101, &isr102, &isr103, &isr104, &isr105, &isr106, &isr107, &isr108, &isr109, &isr110, &isr111,
    &isr112, &isr113, &isr114, &isr115, &isr116, &isr117, &isr118, &isr119, &isr120, &isr121, &isr122, &isr123, &isr124, &isr125, &isr126, &isr127,
    &isr128, &isr129, &isr130, &isr131, &isr132, &isr133, &isr134, &isr135, &isr136, &isr137, &isr138, &isr139, &isr140, &isr141, &isr142, &isr143,
    &isr144, &isr145, &isr146, &isr147, &isr148, &isr149, &isr150, &isr151, &isr152, &isr153, &isr154, &isr155, &isr156, &isr157, &isr158, &isr159,
    &isr160, &isr161, &isr162, &isr163, &isr164, &isr165, &isr166, &isr167, &isr168, &isr169, &isr170, &isr171, &isr172, &isr173, &isr174, &isr175,
    &isr176, &isr177, &isr178, &isr179, &isr180, &isr181, &isr182, &isr183, &isr184, &isr185, &isr186, &isr187, &isr188, &isr189, &isr190, &isr191,
    &isr192, &isr193, &isr194, &isr195, &isr196, &isr197, &isr198, &isr199, &isr200, &isr201, &isr202, &isr203, &isr204, &isr205, &isr206, &isr207,
    &isr208, &isr209, &isr210, &isr211, &isr212, &isr213, &isr214, &isr215, &isr216, &isr217, &isr218, &isr219, &isr220, &isr221, &isr222, &isr223,
    &isr224, &isr225, &isr226, &isr227, &isr228, &isr229, &isr230, &isr231, &isr232, &isr233, &isr234, &isr235, &isr236, &isr237, &isr238, &isr239,
    &isr240, &isr241, &isr242, &isr243, &isr244, &isr245, &isr246, &isr247, &isr248, &isr249, &isr250, &isr251, &isr252, &isr253, &isr254, &isr255,
  };

  IDTEntry idtEntries[IDT_SIZE];
  Handler handlers[IDT_SIZE];

  // Early handlers
  void pageFaultHandler(irq_t irqNumber, uword_t errorCode, uintptr_t oldEip)
  {
    uintptr_t address;
    asm volatile ("mov %[address], cr2" : [address]"=rm"(address) : :);
    rt::logf("\nPage Fault at 0x%lx with error code 0x%lx and old eip 0x%lx\n", address, errorCode, oldEip);
    rt::panic("Page Fault\n");
  }

  void generalProtectionFaultHandler(irq_t irqNumber, uword_t errorCode, uintptr_t oldEip)
  {
    rt::panic("General Protection Fault\n");
  }

  void doubleFaultHandler(irq_t irqNumber, uword_t errorCode, uintptr_t oldEip)
  {
    rt::panic("Double Fault\n");
  }

  void initialize()
  {
    rt::logf("Loading Interrupt Descriptor Table and configuring interrupt handlers...");

    // Setup the idt
    for(size_t i=0; i<IDT_SIZE; ++i)
      idtEntries[i] = IDTEntry(InterruptType::INTERRUPT_GATE_32, PrivilegeLevel::RING0, 0x8, reinterpret_cast<uintptr_t>(isrs[i]));

    // Load idt
    foreachCPUInitCall([](){
      IDT idt(idtEntries);
      asm volatile ( "lidt %[idt]" : : [idt]"m"(idt) : "ax");
    });

    rt::logf("Done\n");
  }

  extern "C" void isr(uword_t irqNumber, uword_t errorCode, uintptr_t oldEip)
  {
    if(!handlers[irqNumber])
    {
      if(irqNumber<0x20)
        rt::panic("Unhandled exception %lu - error code: 0x%lx, old eip: 0x%lx\n", irqNumber, errorCode, oldEip);

      rt::logf("Unhandled interrupts %lu - error code: 0x%lx, old eip: 0x%lx\n", irqNumber, errorCode, oldEip);
      return;
    }

    handlers[irqNumber](irqNumber, errorCode, oldEip);

    tasks::onResume();
  }

  void setKernelStack(uintptr_t ptr, size_t size)
  {
    internals::setKernelStack(0x10, ptr+size);
  }

  void installHandler(int irqNumber, Handler handler, PrivilegeLevel privilegeLevel, bool disableInterrupt)
  {
    // TODO: support chaining of handler via additional void* paramter for user
    //       data
    handlers[irqNumber] = handler;
    auto& idtEntry = idtEntries[irqNumber];
    idtEntry.type(disableInterrupt ? InterruptType::INTERRUPT_GATE_32 : InterruptType::TRAP_GATE_32);
    idtEntry.privilegeLevel(privilegeLevel);
    asm volatile("" : : : "memory");
  }

  void uninstallHandler(int irqNumber)
  {
    handlers[irqNumber] = nullptr;
  }
}
