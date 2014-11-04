/* file "machine/types.h" */

/*
    Copyright (c) 2000 The President and Fellows of Harvard College

    All rights reserved.

    This software is provided under the terms described in
    the "machine/copyright.h" include file.
*/

#ifndef MACHINE_TYPES_H
#define MACHINE_TYPES_H

#include <machine/copyright.h>

#ifdef USE_PRAGMA_INTERFACE
#pragma interface "machine/types.h"
#endif

#include <machine/substrate.h>

extern TypeId type_v0;	// void
extern TypeId type_s8;	// signed ints
extern TypeId type_s16;
extern TypeId type_s32;
extern TypeId type_s64;
extern TypeId type_u8;	// unsigned ints 
extern TypeId type_u16;
extern TypeId type_u32;
extern TypeId type_u64;
extern TypeId type_f32;	// floats
extern TypeId type_f64;
extern TypeId type_f128;

extern TypeId type_p2;	// pointers
extern TypeId type_p4;

extern TypeId type_u1;//zhi: boolean May-13, 2004
extern TypeId type_u2;//zhi:  May-15, 2004
extern TypeId type_u3;//zhi:  May-17, 2004
extern TypeId type_u4;//zhi:  May-15, 2004
extern TypeId type_u5;//zhi:  May-17, 2004
extern TypeId type_u6;//zhi:  May-15, 2004
extern TypeId type_u7;//zhi:  May-17, 2004
extern TypeId type_u9;//zhi:  May-17, 2004
extern TypeId type_u10;//zhi:  May-15, 2004
extern TypeId type_u11;//zhi:  May-17, 2004
extern TypeId type_u12;//zhi:  May-15, 2004
extern TypeId type_u13;//zhi:  May-17, 2004
extern TypeId type_u14;//zhi:  May-15, 2004
extern TypeId type_u15;//zhi:  May-17, 2004
extern TypeId type_u17;//zhi:  May-17, 2004
extern TypeId type_u18;//zhi:  May-15, 2004
extern TypeId type_u19;//zhi:  May-17, 2004
extern TypeId type_u20;//zhi:  May-15, 2004
extern TypeId type_u21;//zhi:  May-17, 2004
extern TypeId type_u22;//zhi:  May-15, 2004
extern TypeId type_u23;//zhi:  May-17, 2004
extern TypeId type_u24;//zhi:  May-15, 2004
extern TypeId type_u25;//zhi:  May-17, 2004
extern TypeId type_u26;//zhi:  May-15, 2004
extern TypeId type_u27;//zhi:  May-17, 2004
extern TypeId type_u28;//zhi:  May-15, 2004
extern TypeId type_u29;//zhi:  May-17, 2004
extern TypeId type_u30;//zhi:  May-15, 2004
extern TypeId type_u31;//zhi:  May-17, 2004

extern TypeId type_s2;//zhi:  May-15, 2004
extern TypeId type_s3;//zhi:  May-17, 2004
extern TypeId type_s4;//zhi:  May-15, 2004
extern TypeId type_s5;//zhi:  May-17, 2004
extern TypeId type_s6;//zhi:  May-15, 2004
extern TypeId type_s7;//zhi:  May-17, 2004
extern TypeId type_s9;//zhi:  May-17, 2004
extern TypeId type_s10;//zhi:  May-15, 2004
extern TypeId type_s11;//zhi:  May-17, 2004
extern TypeId type_s12;//zhi:  May-15, 2004
extern TypeId type_s13;//zhi:  May-17, 2004
extern TypeId type_s14;//zhi:  May-15, 2004
extern TypeId type_s15;//zhi:  May-17, 2004
extern TypeId type_s17;//zhi:  May-17, 2004
extern TypeId type_s18;//zhi:  May-15, 2004
extern TypeId type_s19;//zhi:  May-17, 2004
extern TypeId type_s20;//zhi:  May-15, 2004
extern TypeId type_s21;//zhi:  May-17, 2004
extern TypeId type_s22;//zhi:  May-15, 2004
extern TypeId type_s23;//zhi:  May-17, 2004
extern TypeId type_s24;//zhi:  May-15, 2004
extern TypeId type_s25;//zhi:  May-17, 2004
extern TypeId type_s26;//zhi:  May-15, 2004
extern TypeId type_s27;//zhi:  May-17, 2004
extern TypeId type_s28;//zhi:  May-15, 2004
extern TypeId type_s29;//zhi:  May-17, 2004
extern TypeId type_s30;//zhi:  May-15, 2004
extern TypeId type_s31;//zhi:  May-17, 2004

extern TypeId type_p8;
extern TypeId type_p9;
extern TypeId type_p10;
extern TypeId type_p11;
extern TypeId type_p12;
extern TypeId type_p13;
extern TypeId type_p14;
extern TypeId type_p15;
extern TypeId type_p16;
extern TypeId type_p17;
extern TypeId type_p18;
extern TypeId type_p19;
extern TypeId type_p20;
extern TypeId type_p21;
extern TypeId type_p22;
extern TypeId type_p23;
extern TypeId type_p24;
extern TypeId type_p25;
extern TypeId type_p26;
extern TypeId type_p27;
extern TypeId type_p28;
extern TypeId type_p29;
extern TypeId type_p30;
extern TypeId type_p31;
extern TypeId type_p32;
extern TypeId type_p33;
extern TypeId type_p34;
extern TypeId type_p35;
extern TypeId type_p36;
extern TypeId type_p37;
extern TypeId type_p38;
extern TypeId type_p39;
extern TypeId type_p40;
extern TypeId type_p41;
extern TypeId type_p42;
extern TypeId type_p43;
extern TypeId type_p44;
extern TypeId type_p45;
extern TypeId type_p46;
extern TypeId type_p47;
extern TypeId type_p48;
extern TypeId type_p49;
extern TypeId type_p50;
extern TypeId type_p51;
extern TypeId type_p52;
extern TypeId type_p53;
extern TypeId type_p54;
extern TypeId type_p55;
extern TypeId type_p56;
extern TypeId type_p57;
extern TypeId type_p58;
extern TypeId type_p59;
extern TypeId type_p60;
extern TypeId type_p61;
extern TypeId type_p62;
extern TypeId type_p63;
extern TypeId type_p64;
extern TypeId type_p65;
extern TypeId type_p66;
extern TypeId type_p67;
extern TypeId type_p68;
extern TypeId type_p69;
extern TypeId type_p70;
extern TypeId type_p71;
extern TypeId type_p72;
extern TypeId type_p73;
extern TypeId type_p74;
extern TypeId type_p75;
extern TypeId type_p76;
extern TypeId type_p77;
extern TypeId type_p78;
extern TypeId type_p79;
extern TypeId type_p80;
extern TypeId type_p81;
extern TypeId type_p82;
extern TypeId type_p83;
extern TypeId type_p84;
extern TypeId type_p85;
extern TypeId type_p86;
extern TypeId type_p87;
extern TypeId type_p88;
extern TypeId type_p89;
extern TypeId type_p90;
extern TypeId type_p91;
extern TypeId type_p92;
extern TypeId type_p93;
extern TypeId type_p94;
extern TypeId type_p95;
extern TypeId type_p96;
extern TypeId type_p97;
extern TypeId type_p98;
extern TypeId type_p99;
extern TypeId type_p100;
extern TypeId type_p101;
extern TypeId type_p102;
extern TypeId type_p103;
extern TypeId type_p104;
extern TypeId type_p105;
extern TypeId type_p106;
extern TypeId type_p107;
extern TypeId type_p108;
extern TypeId type_p109;
extern TypeId type_p110;
extern TypeId type_p111;
extern TypeId type_p112;
extern TypeId type_p113;
extern TypeId type_p114;
extern TypeId type_p115;
extern TypeId type_p116;
extern TypeId type_p117;
extern TypeId type_p118;
extern TypeId type_p119;
extern TypeId type_p120;
extern TypeId type_p121;
extern TypeId type_p122;
extern TypeId type_p123;
extern TypeId type_p124;
extern TypeId type_p125;
extern TypeId type_p126;
extern TypeId type_p127;
extern TypeId type_p128;
extern TypeId type_p129;
extern TypeId type_p130;
extern TypeId type_p131;
extern TypeId type_p132;
extern TypeId type_p133;
extern TypeId type_p134;
extern TypeId type_p135;
extern TypeId type_p136;
extern TypeId type_p137;
extern TypeId type_p138;
extern TypeId type_p139;
extern TypeId type_p140;
extern TypeId type_p141;
extern TypeId type_p142;
extern TypeId type_p143;
extern TypeId type_p144;
extern TypeId type_p145;
extern TypeId type_p146;
extern TypeId type_p147;
extern TypeId type_p148;
extern TypeId type_p149;
extern TypeId type_p150;
extern TypeId type_p151;
extern TypeId type_p152;
extern TypeId type_p153;
extern TypeId type_p154;
extern TypeId type_p155;
extern TypeId type_p156;
extern TypeId type_p157;
extern TypeId type_p158;
extern TypeId type_p159;
extern TypeId type_p160;
extern TypeId type_p161;
extern TypeId type_p162;
extern TypeId type_p163;
extern TypeId type_p164;
extern TypeId type_p165;
extern TypeId type_p166;
extern TypeId type_p167;
extern TypeId type_p168;
extern TypeId type_p169;
extern TypeId type_p170;
extern TypeId type_p171;
extern TypeId type_p172;
extern TypeId type_p173;
extern TypeId type_p174;
extern TypeId type_p175;
extern TypeId type_p176;
extern TypeId type_p177;
extern TypeId type_p178;
extern TypeId type_p179;
extern TypeId type_p180;
extern TypeId type_p181;
extern TypeId type_p182;
extern TypeId type_p183;
extern TypeId type_p184;
extern TypeId type_p185;
extern TypeId type_p186;
extern TypeId type_p187;
extern TypeId type_p188;
extern TypeId type_p189;
extern TypeId type_p190;
extern TypeId type_p191;
extern TypeId type_p192;
extern TypeId type_p193;
extern TypeId type_p194;
extern TypeId type_p195;
extern TypeId type_p196;
extern TypeId type_p197;
extern TypeId type_p198;
extern TypeId type_p199;
extern TypeId type_p200;
extern TypeId type_p201;
extern TypeId type_p202;
extern TypeId type_p203;
extern TypeId type_p204;
extern TypeId type_p205;
extern TypeId type_p206;
extern TypeId type_p207;
extern TypeId type_p208;
extern TypeId type_p209;
extern TypeId type_p210;
extern TypeId type_p211;
extern TypeId type_p212;
extern TypeId type_p213;
extern TypeId type_p214;
extern TypeId type_p215;
extern TypeId type_p216;
extern TypeId type_p217;
extern TypeId type_p218;
extern TypeId type_p219;
extern TypeId type_p220;
extern TypeId type_p221;
extern TypeId type_p222;
extern TypeId type_p223;
extern TypeId type_p224;
extern TypeId type_p225;
extern TypeId type_p226;
extern TypeId type_p227;
extern TypeId type_p228;
extern TypeId type_p229;
extern TypeId type_p230;
extern TypeId type_p231;
extern TypeId type_p232;
extern TypeId type_p233;
extern TypeId type_p234;
extern TypeId type_p235;
extern TypeId type_p236;
extern TypeId type_p237;
extern TypeId type_p238;
extern TypeId type_p239;
extern TypeId type_p240;
extern TypeId type_p241;
extern TypeId type_p242;
extern TypeId type_p243;
extern TypeId type_p244;
extern TypeId type_p245;
extern TypeId type_p246;
extern TypeId type_p247;
extern TypeId type_p248;
extern TypeId type_p249;
extern TypeId type_p250;
extern TypeId type_p251;
extern TypeId type_p252;
extern TypeId type_p253;
extern TypeId type_p254;
extern TypeId type_p255;
extern TypeId type_p256;
extern TypeId type_p312;
extern TypeId type_p512;
extern TypeId type_p300;
extern TypeId type_p600;
extern TypeId type_p352;
extern TypeId type_p704;

extern TypeId type_p1;	// pointers
extern TypeId type_p3;	// pointers
extern TypeId type_p5;	// pointers
extern TypeId type_p6;	// pointers
extern TypeId type_p7;	// pointers

void attach_opi_predefined_types(FileSetBlock*);
void set_opi_predefined_types(FileSetBlock*);

#define type_ptr type_addr()

TypeId type_addr();

void fprint(FILE*, TypeId);

enum TYPEID_enum_type {
    TYPEID_NONE = 2,
    TYPEID_VOID,
    TYPEID_SINT, 
    TYPEID_UINT, 
    TYPEID_FLOAT,
    TYPEID_PTR, 
    TYPEID_BOOLEAN
};

TypeId build_typeid_type(TYPEID_enum_type et, int bitsize = 0);

#endif /* MACHINE_TYPES_H */
