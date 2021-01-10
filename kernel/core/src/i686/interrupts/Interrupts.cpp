#include <core/i686/interrupts/Interrupts.hpp>

#include <core/i686/interrupts/IDT.hpp>
#include <core/i686/interrupts/ISR.hpp>

#include <core/i686/internals/Segmentation.hpp>


#include <core/generic/Init.hpp>
#include <core/generic/Panic.hpp>
#include <core/generic/io/Print.hpp>

#include <stddef.h>
#include <assert.h>

namespace core::interrupts
{
  constexpr static size_t IDT_SIZE = 256;
  IDTEntry idtEntries[IDT_SIZE];

  uintptr_t isrs[IDT_SIZE] = {
    reinterpret_cast<uintptr_t>(&isr0),   reinterpret_cast<uintptr_t>(&isr1),   reinterpret_cast<uintptr_t>(&isr2),   reinterpret_cast<uintptr_t>(&isr3),   reinterpret_cast<uintptr_t>(&isr4),   reinterpret_cast<uintptr_t>(&isr5),   reinterpret_cast<uintptr_t>(&isr6),   reinterpret_cast<uintptr_t>(&isr7),
    reinterpret_cast<uintptr_t>(&isr8),   reinterpret_cast<uintptr_t>(&isr9),   reinterpret_cast<uintptr_t>(&isr10),  reinterpret_cast<uintptr_t>(&isr11),  reinterpret_cast<uintptr_t>(&isr12),  reinterpret_cast<uintptr_t>(&isr13),  reinterpret_cast<uintptr_t>(&isr14),  reinterpret_cast<uintptr_t>(&isr15),
    reinterpret_cast<uintptr_t>(&isr16),  reinterpret_cast<uintptr_t>(&isr17),  reinterpret_cast<uintptr_t>(&isr18),  reinterpret_cast<uintptr_t>(&isr19),  reinterpret_cast<uintptr_t>(&isr20),  reinterpret_cast<uintptr_t>(&isr21),  reinterpret_cast<uintptr_t>(&isr22),  reinterpret_cast<uintptr_t>(&isr23),
    reinterpret_cast<uintptr_t>(&isr24),  reinterpret_cast<uintptr_t>(&isr25),  reinterpret_cast<uintptr_t>(&isr26),  reinterpret_cast<uintptr_t>(&isr27),  reinterpret_cast<uintptr_t>(&isr28),  reinterpret_cast<uintptr_t>(&isr29),  reinterpret_cast<uintptr_t>(&isr30),  reinterpret_cast<uintptr_t>(&isr31),
    reinterpret_cast<uintptr_t>(&isr32),  reinterpret_cast<uintptr_t>(&isr33),  reinterpret_cast<uintptr_t>(&isr34),  reinterpret_cast<uintptr_t>(&isr35),  reinterpret_cast<uintptr_t>(&isr36),  reinterpret_cast<uintptr_t>(&isr37),  reinterpret_cast<uintptr_t>(&isr38),  reinterpret_cast<uintptr_t>(&isr39),
    reinterpret_cast<uintptr_t>(&isr40),  reinterpret_cast<uintptr_t>(&isr41),  reinterpret_cast<uintptr_t>(&isr42),  reinterpret_cast<uintptr_t>(&isr43),  reinterpret_cast<uintptr_t>(&isr44),  reinterpret_cast<uintptr_t>(&isr45),  reinterpret_cast<uintptr_t>(&isr46),  reinterpret_cast<uintptr_t>(&isr47),
    reinterpret_cast<uintptr_t>(&isr48),  reinterpret_cast<uintptr_t>(&isr49),  reinterpret_cast<uintptr_t>(&isr50),  reinterpret_cast<uintptr_t>(&isr51),  reinterpret_cast<uintptr_t>(&isr52),  reinterpret_cast<uintptr_t>(&isr53),  reinterpret_cast<uintptr_t>(&isr54),  reinterpret_cast<uintptr_t>(&isr55),
    reinterpret_cast<uintptr_t>(&isr56),  reinterpret_cast<uintptr_t>(&isr57),  reinterpret_cast<uintptr_t>(&isr58),  reinterpret_cast<uintptr_t>(&isr59),  reinterpret_cast<uintptr_t>(&isr60),  reinterpret_cast<uintptr_t>(&isr61),  reinterpret_cast<uintptr_t>(&isr62),  reinterpret_cast<uintptr_t>(&isr63),
    reinterpret_cast<uintptr_t>(&isr64),  reinterpret_cast<uintptr_t>(&isr65),  reinterpret_cast<uintptr_t>(&isr66),  reinterpret_cast<uintptr_t>(&isr67),  reinterpret_cast<uintptr_t>(&isr68),  reinterpret_cast<uintptr_t>(&isr69),  reinterpret_cast<uintptr_t>(&isr70),  reinterpret_cast<uintptr_t>(&isr71),
    reinterpret_cast<uintptr_t>(&isr72),  reinterpret_cast<uintptr_t>(&isr73),  reinterpret_cast<uintptr_t>(&isr74),  reinterpret_cast<uintptr_t>(&isr75),  reinterpret_cast<uintptr_t>(&isr76),  reinterpret_cast<uintptr_t>(&isr77),  reinterpret_cast<uintptr_t>(&isr78),  reinterpret_cast<uintptr_t>(&isr79),
    reinterpret_cast<uintptr_t>(&isr80),  reinterpret_cast<uintptr_t>(&isr81),  reinterpret_cast<uintptr_t>(&isr82),  reinterpret_cast<uintptr_t>(&isr83),  reinterpret_cast<uintptr_t>(&isr84),  reinterpret_cast<uintptr_t>(&isr85),  reinterpret_cast<uintptr_t>(&isr86),  reinterpret_cast<uintptr_t>(&isr87),
    reinterpret_cast<uintptr_t>(&isr88),  reinterpret_cast<uintptr_t>(&isr89),  reinterpret_cast<uintptr_t>(&isr90),  reinterpret_cast<uintptr_t>(&isr91),  reinterpret_cast<uintptr_t>(&isr92),  reinterpret_cast<uintptr_t>(&isr93),  reinterpret_cast<uintptr_t>(&isr94),  reinterpret_cast<uintptr_t>(&isr95),
    reinterpret_cast<uintptr_t>(&isr96),  reinterpret_cast<uintptr_t>(&isr97),  reinterpret_cast<uintptr_t>(&isr98),  reinterpret_cast<uintptr_t>(&isr99),  reinterpret_cast<uintptr_t>(&isr100), reinterpret_cast<uintptr_t>(&isr101), reinterpret_cast<uintptr_t>(&isr102), reinterpret_cast<uintptr_t>(&isr103),
    reinterpret_cast<uintptr_t>(&isr104), reinterpret_cast<uintptr_t>(&isr105), reinterpret_cast<uintptr_t>(&isr106), reinterpret_cast<uintptr_t>(&isr107), reinterpret_cast<uintptr_t>(&isr108), reinterpret_cast<uintptr_t>(&isr109), reinterpret_cast<uintptr_t>(&isr110), reinterpret_cast<uintptr_t>(&isr111),
    reinterpret_cast<uintptr_t>(&isr112), reinterpret_cast<uintptr_t>(&isr113), reinterpret_cast<uintptr_t>(&isr114), reinterpret_cast<uintptr_t>(&isr115), reinterpret_cast<uintptr_t>(&isr116), reinterpret_cast<uintptr_t>(&isr117), reinterpret_cast<uintptr_t>(&isr118), reinterpret_cast<uintptr_t>(&isr119),
    reinterpret_cast<uintptr_t>(&isr120), reinterpret_cast<uintptr_t>(&isr121), reinterpret_cast<uintptr_t>(&isr122), reinterpret_cast<uintptr_t>(&isr123), reinterpret_cast<uintptr_t>(&isr124), reinterpret_cast<uintptr_t>(&isr125), reinterpret_cast<uintptr_t>(&isr126), reinterpret_cast<uintptr_t>(&isr127),
    reinterpret_cast<uintptr_t>(&isr128), reinterpret_cast<uintptr_t>(&isr129), reinterpret_cast<uintptr_t>(&isr130), reinterpret_cast<uintptr_t>(&isr131), reinterpret_cast<uintptr_t>(&isr132), reinterpret_cast<uintptr_t>(&isr133), reinterpret_cast<uintptr_t>(&isr134), reinterpret_cast<uintptr_t>(&isr135),
    reinterpret_cast<uintptr_t>(&isr136), reinterpret_cast<uintptr_t>(&isr137), reinterpret_cast<uintptr_t>(&isr138), reinterpret_cast<uintptr_t>(&isr139), reinterpret_cast<uintptr_t>(&isr140), reinterpret_cast<uintptr_t>(&isr141), reinterpret_cast<uintptr_t>(&isr142), reinterpret_cast<uintptr_t>(&isr143),
    reinterpret_cast<uintptr_t>(&isr144), reinterpret_cast<uintptr_t>(&isr145), reinterpret_cast<uintptr_t>(&isr146), reinterpret_cast<uintptr_t>(&isr147), reinterpret_cast<uintptr_t>(&isr148), reinterpret_cast<uintptr_t>(&isr149), reinterpret_cast<uintptr_t>(&isr150), reinterpret_cast<uintptr_t>(&isr151),
    reinterpret_cast<uintptr_t>(&isr152), reinterpret_cast<uintptr_t>(&isr153), reinterpret_cast<uintptr_t>(&isr154), reinterpret_cast<uintptr_t>(&isr155), reinterpret_cast<uintptr_t>(&isr156), reinterpret_cast<uintptr_t>(&isr157), reinterpret_cast<uintptr_t>(&isr158), reinterpret_cast<uintptr_t>(&isr159),
    reinterpret_cast<uintptr_t>(&isr160), reinterpret_cast<uintptr_t>(&isr161), reinterpret_cast<uintptr_t>(&isr162), reinterpret_cast<uintptr_t>(&isr163), reinterpret_cast<uintptr_t>(&isr164), reinterpret_cast<uintptr_t>(&isr165), reinterpret_cast<uintptr_t>(&isr166), reinterpret_cast<uintptr_t>(&isr167),
    reinterpret_cast<uintptr_t>(&isr168), reinterpret_cast<uintptr_t>(&isr169), reinterpret_cast<uintptr_t>(&isr170), reinterpret_cast<uintptr_t>(&isr171), reinterpret_cast<uintptr_t>(&isr172), reinterpret_cast<uintptr_t>(&isr173), reinterpret_cast<uintptr_t>(&isr174), reinterpret_cast<uintptr_t>(&isr175),
    reinterpret_cast<uintptr_t>(&isr176), reinterpret_cast<uintptr_t>(&isr177), reinterpret_cast<uintptr_t>(&isr178), reinterpret_cast<uintptr_t>(&isr179), reinterpret_cast<uintptr_t>(&isr180), reinterpret_cast<uintptr_t>(&isr181), reinterpret_cast<uintptr_t>(&isr182), reinterpret_cast<uintptr_t>(&isr183),
    reinterpret_cast<uintptr_t>(&isr184), reinterpret_cast<uintptr_t>(&isr185), reinterpret_cast<uintptr_t>(&isr186), reinterpret_cast<uintptr_t>(&isr187), reinterpret_cast<uintptr_t>(&isr188), reinterpret_cast<uintptr_t>(&isr189), reinterpret_cast<uintptr_t>(&isr190), reinterpret_cast<uintptr_t>(&isr191),
    reinterpret_cast<uintptr_t>(&isr192), reinterpret_cast<uintptr_t>(&isr193), reinterpret_cast<uintptr_t>(&isr194), reinterpret_cast<uintptr_t>(&isr195), reinterpret_cast<uintptr_t>(&isr196), reinterpret_cast<uintptr_t>(&isr197), reinterpret_cast<uintptr_t>(&isr198), reinterpret_cast<uintptr_t>(&isr199),
    reinterpret_cast<uintptr_t>(&isr200), reinterpret_cast<uintptr_t>(&isr201), reinterpret_cast<uintptr_t>(&isr202), reinterpret_cast<uintptr_t>(&isr203), reinterpret_cast<uintptr_t>(&isr204), reinterpret_cast<uintptr_t>(&isr205), reinterpret_cast<uintptr_t>(&isr206), reinterpret_cast<uintptr_t>(&isr207),
    reinterpret_cast<uintptr_t>(&isr208), reinterpret_cast<uintptr_t>(&isr209), reinterpret_cast<uintptr_t>(&isr210), reinterpret_cast<uintptr_t>(&isr211), reinterpret_cast<uintptr_t>(&isr212), reinterpret_cast<uintptr_t>(&isr213), reinterpret_cast<uintptr_t>(&isr214), reinterpret_cast<uintptr_t>(&isr215),
    reinterpret_cast<uintptr_t>(&isr216), reinterpret_cast<uintptr_t>(&isr217), reinterpret_cast<uintptr_t>(&isr218), reinterpret_cast<uintptr_t>(&isr219), reinterpret_cast<uintptr_t>(&isr220), reinterpret_cast<uintptr_t>(&isr221), reinterpret_cast<uintptr_t>(&isr222), reinterpret_cast<uintptr_t>(&isr223),
    reinterpret_cast<uintptr_t>(&isr224), reinterpret_cast<uintptr_t>(&isr225), reinterpret_cast<uintptr_t>(&isr226), reinterpret_cast<uintptr_t>(&isr227), reinterpret_cast<uintptr_t>(&isr228), reinterpret_cast<uintptr_t>(&isr229), reinterpret_cast<uintptr_t>(&isr230), reinterpret_cast<uintptr_t>(&isr231),
    reinterpret_cast<uintptr_t>(&isr232), reinterpret_cast<uintptr_t>(&isr233), reinterpret_cast<uintptr_t>(&isr234), reinterpret_cast<uintptr_t>(&isr235), reinterpret_cast<uintptr_t>(&isr236), reinterpret_cast<uintptr_t>(&isr237), reinterpret_cast<uintptr_t>(&isr238), reinterpret_cast<uintptr_t>(&isr239),
    reinterpret_cast<uintptr_t>(&isr240), reinterpret_cast<uintptr_t>(&isr241), reinterpret_cast<uintptr_t>(&isr242), reinterpret_cast<uintptr_t>(&isr243), reinterpret_cast<uintptr_t>(&isr244), reinterpret_cast<uintptr_t>(&isr245), reinterpret_cast<uintptr_t>(&isr246), reinterpret_cast<uintptr_t>(&isr247),
    reinterpret_cast<uintptr_t>(&isr248), reinterpret_cast<uintptr_t>(&isr249), reinterpret_cast<uintptr_t>(&isr250), reinterpret_cast<uintptr_t>(&isr251), reinterpret_cast<uintptr_t>(&isr252), reinterpret_cast<uintptr_t>(&isr253), reinterpret_cast<uintptr_t>(&isr254), reinterpret_cast<uintptr_t>(&isr255)
  };
  Handler handlers[IDT_SIZE] = {};

  void nullHandler(uint8_t irqNumber, uint32_t errorCode, uintptr_t oldEip)
  {
    io::printf("Unhandled Interrupt %u\n", static_cast<unsigned>(irqNumber));
    for(;;) asm volatile("hlt");
  }

  void pageFaultHandler(uint8_t irqNumber, uint32_t errorCode, uintptr_t oldEip)
  {
    uint32_t address;
    asm volatile ("mov %[address], cr2" : [address]"=rm"(address) : :);
    io::printf("\nPage Fault at %lx with error code %lx and old eip %lx\n", address, errorCode, oldEip);
    panic("Page Fault\n");
  }

  INIT_FUNCTION_EARLY void initInterrupts()
  {
    io::print("Loading Interrupt Descriptor Table and configuring interrupt handlers...");

    // Load idt
    IDT idt(idtEntries);
    asm volatile ( "lidt %[idt]" : : [idt]"m"(idt) : "ax");

    for(size_t i=0; i<IDT_SIZE; ++i)
      uninstallHandler(i);

    installHandler(8,  [](uint8_t, uint32_t, uintptr_t)   { panic("Double Fault\n"); },       PrivilegeLevel::RING0, true);
    installHandler(13, [](uint8_t, uint32_t, uintptr_t)   { panic("Protection Fault\n"); },   PrivilegeLevel::RING0, true);
    installHandler(14, &pageFaultHandler,                                          PrivilegeLevel::RING0, true);
    installHandler(0x80, [](uint8_t, uint32_t, uintptr_t) { io::print("User Interrupt\n"); }, PrivilegeLevel::RING3, true);

    asm volatile("" : : : "memory");
    io::print("Done\n");
  }

  extern "C" void isr(uint32_t irqNumber, uint32_t errorCode, uintptr_t oldEip)
  {
    handlers[irqNumber](irqNumber, errorCode, oldEip);
  }

  void setKernelStack(uintptr_t ptr, size_t size)
  {
    internals::tss.ss0  = 0x10;
    internals::tss.esp0 = ptr + size;
  }

  void installHandler(int irqNumber, Handler handler, PrivilegeLevel privilegeLevel, bool disableInterrupt)
  {
    // TODO: support chaining of handler via additional void* paramter for user
    //       data
    handlers[irqNumber] = handler;
    auto interruptType = disableInterrupt ? InterruptType::INTERRUPT_GATE_32 : InterruptType::TRAP_GATE_32;
    idtEntries[irqNumber] = IDTEntry(interruptType, privilegeLevel, 0x8, isrs[irqNumber]);
  }

  void uninstallHandler(int irqNumber)
  {
    installHandler(irqNumber, &nullHandler, PrivilegeLevel::RING0, true);
  }
}
