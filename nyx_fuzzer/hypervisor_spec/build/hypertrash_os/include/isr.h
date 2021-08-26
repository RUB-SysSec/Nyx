/*
 * HyperCube OS
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */ 

#pragma once 

#define UNUSED_VAR     __attribute__ ((unused))

uint8_t alloc_msi_vectors(uint8_t vnum);

/*
 * Exception Handlers
 */
extern void _isr0();
extern void _isr1();
extern void _isr2();
extern void _isr3();
extern void _isr4();
extern void _isr5();
extern void _isr6();
extern void _isr7();
extern void _isr8();
extern void _isr9();
extern void _isr10();
extern void _isr11();
extern void _isr12();
extern void _isr13();
extern void _isr14();
extern void _isr15();
extern void _isr16();
extern void _isr17();
extern void _isr18();
extern void _isr19();
extern void _isr20();
extern void _isr21();
extern void _isr22();
extern void _isr23();
extern void _isr24();
extern void _isr25();
extern void _isr26();
extern void _isr27();
extern void _isr28();
extern void _isr29();
extern void _isr30();
extern void _isr31();

/* IOPAIC Interrupt Handlers */
extern void _irq0();
extern void _irq1();
extern void _irq2();
extern void _irq3();
extern void _irq4();
extern void _irq5();
extern void _irq6();
extern void _irq7();
extern void _irq8();
extern void _irq9();
extern void _irq10();
extern void _irq11();
extern void _irq12();
extern void _irq13();
extern void _irq14();
extern void _irq15();
extern void _irq16();
extern void _irq17();
extern void _irq18();
extern void _irq19();
extern void _irq20();
extern void _irq21();
extern void _irq22();
extern void _irq23();

/* MSI Interrupts */ 
extern void _irq24();
extern void _irq25();
extern void _irq26();
extern void _irq27();
extern void _irq28();
extern void _irq29();
extern void _irq30();
extern void _irq31();
extern void _irq32();
extern void _irq33();
extern void _irq34();
extern void _irq35();
extern void _irq36();
extern void _irq37();
extern void _irq38();
extern void _irq39();
extern void _irq40();
extern void _irq41();
extern void _irq42();
extern void _irq43();
extern void _irq44();
extern void _irq45();
extern void _irq46();
extern void _irq47();
extern void _irq48();
extern void _irq49();
extern void _irq50();
extern void _irq51();
extern void _irq52();
extern void _irq53();
extern void _irq54();
extern void _irq55();
extern void _irq56();
extern void _irq57();
extern void _irq58();
extern void _irq59();
extern void _irq60();
extern void _irq61();
extern void _irq62();
extern void _irq63();
extern void _irq64();
extern void _irq65();
extern void _irq66();
extern void _irq67();
extern void _irq68();
extern void _irq69();
extern void _irq70();
extern void _irq71();
extern void _irq72();
extern void _irq73();
extern void _irq74();
extern void _irq75();
extern void _irq76();
extern void _irq77();
extern void _irq78();
extern void _irq79();
extern void _irq80();
extern void _irq81();
extern void _irq82();
extern void _irq83();
extern void _irq84();
extern void _irq85();
extern void _irq86();
extern void _irq87();
extern void _irq88();
extern void _irq89();
extern void _irq90();
extern void _irq91();
extern void _irq92();
extern void _irq93();
extern void _irq94();
extern void _irq95();
extern void _irq96();
extern void _irq97();
extern void _irq98();
extern void _irq99();
extern void _irq100();
extern void _irq101();
extern void _irq102();
extern void _irq103();
extern void _irq104();
extern void _irq105();
extern void _irq106();
extern void _irq107();
extern void _irq108();
extern void _irq109();
extern void _irq110();
extern void _irq111();
extern void _irq112();
extern void _irq113();
extern void _irq114();
extern void _irq115();
extern void _irq116();
extern void _irq117();
extern void _irq118();
extern void _irq119();
extern void _irq120();
extern void _irq121();
extern void _irq122();
extern void _irq123();
extern void _irq124();
extern void _irq125();
extern void _irq126();
extern void _irq127();
extern void _irq128();
extern void _irq129();
extern void _irq130();
extern void _irq131();
extern void _irq132();
extern void _irq133();
extern void _irq134();
extern void _irq135();
extern void _irq136();
extern void _irq137();
extern void _irq138();
extern void _irq139();
extern void _irq140();
extern void _irq141();
extern void _irq142();
extern void _irq143();
extern void _irq144();
extern void _irq145();
extern void _irq146();
extern void _irq147();
extern void _irq148();
extern void _irq149();
extern void _irq150();
extern void _irq151();
extern void _irq152();
extern void _irq153();
extern void _irq154();
extern void _irq155();
extern void _irq156();
extern void _irq157();
extern void _irq158();
extern void _irq159();
extern void _irq160();
extern void _irq161();
extern void _irq162();
extern void _irq163();
extern void _irq164();
extern void _irq165();
extern void _irq166();
extern void _irq167();
extern void _irq168();
extern void _irq169();
extern void _irq170();
extern void _irq171();
extern void _irq172();
extern void _irq173();
extern void _irq174();
extern void _irq175();
extern void _irq176();
extern void _irq177();
extern void _irq178();
extern void _irq179();
extern void _irq180();
extern void _irq181();
extern void _irq182();
extern void _irq183();
extern void _irq184();
extern void _irq185();
extern void _irq186();
extern void _irq187();
extern void _irq188();
extern void _irq189();
extern void _irq190();
extern void _irq191();
extern void _irq192();
extern void _irq193();
extern void _irq194();
extern void _irq195();
extern void _irq196();
extern void _irq197();
extern void _irq198();
extern void _irq199();
extern void _irq200();
extern void _irq201();
extern void _irq202();
extern void _irq203();
extern void _irq204();
extern void _irq205();
extern void _irq206();
extern void _irq207();
extern void _irq208();
extern void _irq209();
extern void _irq210();
extern void _irq211();
extern void _irq212();
extern void _irq213();
extern void _irq214();
extern void _irq215();
extern void _irq216();
extern void _irq217();
extern void _irq218();
extern void _irq219();
extern void _irq220();
extern void _irq221();
extern void _irq222();
extern void _irq_spurious();

UNUSED_VAR static void (*isrs[])(void) = {
	_isr0,
	_isr1,
	_isr2,
	_isr3,
	_isr4,
	_isr5,
	_isr6,
	_isr7,
	_isr8,
	_isr9,
	_isr10,
	_isr11,
	_isr12,
	_isr13,
	_isr14,
	_isr15,
	_isr16,
	_isr17,
	_isr18,
	_isr19,
	_isr20,
	_isr21,
	_isr22,
	_isr23,
	_isr24,
	_isr25,
	_isr26,
	_isr27,
	_isr28,
	_isr29,
	_isr30,
	_isr31
};

UNUSED_VAR static void (*irqs[])(void) = {
	_irq0,
	_irq1,
	_irq2,
	_irq3,
	_irq4,
	_irq5,
	_irq6,
	_irq7,
	_irq8,
	_irq9,
	_irq10,
	_irq11,
	_irq12,
	_irq13,
	_irq14,
	_irq15,
	_irq16,
	_irq17,
	_irq18,
	_irq19,
	_irq20,
	_irq21,
	_irq22,
	_irq23
};

UNUSED_VAR static void (*msi_irqs[])(void) = {
	_irq24,
	_irq25,
	_irq26,
	_irq27,
	_irq28,
	_irq29,
	_irq30,
	_irq31,
	_irq32,
	_irq33,
	_irq34,
	_irq35,
	_irq36,
	_irq37,
	_irq38,
	_irq39,
	_irq40,
	_irq41,
	_irq42,
	_irq43,
	_irq44,
	_irq45,
	_irq46,
	_irq47,
	_irq48,
	_irq49,
	_irq50,
	_irq51,
	_irq52,
	_irq53,
	_irq54,
	_irq55,
	_irq56,
	_irq57,
	_irq58,
	_irq59,
	_irq60,
	_irq61,
	_irq62,
	_irq63,
	_irq64,
	_irq65,
	_irq66,
	_irq67,
	_irq68,
	_irq69,
	_irq70,
	_irq71,
	_irq72,
	_irq73,
	_irq74,
	_irq75,
	_irq76,
	_irq77,
	_irq78,
	_irq79,
	_irq80,
	_irq81,
	_irq82,
	_irq83,
	_irq84,
	_irq85,
	_irq86,
	_irq87,
	_irq88,
	_irq89,
	_irq90,
	_irq91,
	_irq92,
	_irq93,
	_irq94,
	_irq95,
	_irq96,
	_irq97,
	_irq98,
	_irq99,
	_irq100,
	_irq101,
	_irq102,
	_irq103,
	_irq104,
	_irq105,
	_irq106,
	_irq107,
	_irq108,
	_irq109,
	_irq110,
	_irq111,
	_irq112,
	_irq113,
	_irq114,
	_irq115,
	_irq116,
	_irq117,
	_irq118,
	_irq119,
	_irq120,
	_irq121,
	_irq122,
	_irq123,
	_irq124,
	_irq125,
	_irq126,
	_irq127,
	_irq128,
	_irq129,
	_irq130,
	_irq131,
	_irq132,
	_irq133,
	_irq134,
	_irq135,
	_irq136,
	_irq137,
	_irq138,
	_irq139,
	_irq140,
	_irq141,
	_irq142,
	_irq143,
	_irq144,
	_irq145,
	_irq146,
	_irq147,
	_irq148,
	_irq149,
	_irq150,
	_irq151,
	_irq152,
	_irq153,
	_irq154,
	_irq155,
	_irq156,
	_irq157,
	_irq158,
	_irq159,
	_irq160,
	_irq161,
	_irq162,
	_irq163,
	_irq164,
	_irq165,
	_irq166,
	_irq167,
	_irq168,
	_irq169,
	_irq170,
	_irq171,
	_irq172,
	_irq173,
	_irq174,
	_irq175,
	_irq176,
	_irq177,
	_irq178,
	_irq179,
	_irq180,
	_irq181,
	_irq182,
	_irq183,
	_irq184,
	_irq185,
	_irq186,
	_irq187,
	_irq188,
	_irq189,
	_irq190,
	_irq191,
	_irq192,
	_irq193,
	_irq194,
	_irq195,
	_irq196,
	_irq197,
	_irq198,
	_irq199,
	_irq200,
	_irq201,
	_irq202,
	_irq203,
	_irq204,
	_irq205,
	_irq206,
	_irq207,
	_irq208,
	_irq209,
	_irq210,
	_irq211,
	_irq212,
	_irq213,
	_irq214,
	_irq215,
	_irq216,
	_irq217,
	_irq218,
	_irq219,
	_irq220,
	_irq221,
	_irq222
};
