/* file "machine/types.cpp" */

/*
   Copyright (c) 2000 The President and Fellows of Harvard College

   All rights reserved.

   This software is provided under the terms described in
   the "machine/copyright.h" include file.
   */

#include <machine/copyright.h>


#ifdef USE_PRAGMA_INTERFACE
#pragma implementation "machine/types.h"
#endif

#include <machine/substrate.h>
#include <machine/init.h>
#include <machine/problems.h>
#include <machine/contexts.h>
#include <machine/util.h>
#include <machine/types.h>


#ifdef USE_DMALLOC
#include <dmalloc.h>
#define new D_NEW
#endif

TypeId type_v0;			// void
TypeId type_s8;			// signed ints
TypeId type_s16;
TypeId type_s32;
TypeId type_s64;
TypeId type_u8;			// unsigned ints
TypeId type_u16;
TypeId type_u32;
TypeId type_u64;
TypeId type_f32;		// floats
TypeId type_f64;
TypeId type_f128;
TypeId type_p2;		// pointers
TypeId type_p4;
TypeId type_u1;//zhi: boolean May-12, 2004
TypeId type_u2;//zhi:  May-15, 2004
TypeId type_u3;//zhi:  May-17, 2004
TypeId type_u4;//zhi:  May-15, 2004
TypeId type_u5;//zhi:  May-17, 2004
TypeId type_u6;//zhi:  May-15, 2004
TypeId type_u7;//zhi:  May-17, 2004
TypeId type_u9;//zhi:  May-17, 2004
TypeId type_u10;//zhi:  May-15, 2004
TypeId type_u11;//zhi:  May-17, 2004
TypeId type_u12;//zhi:  May-15, 2004
TypeId type_u13;//zhi:  May-17, 2004
TypeId type_u14;//zhi:  May-15, 2004
TypeId type_u15;//zhi:  May-17, 2004
TypeId type_u17;//zhi:  May-17, 2004
TypeId type_u18;//zhi:  May-15, 2004
TypeId type_u19;//zhi:  May-17, 2004
TypeId type_u20;//zhi:  May-15, 2004
TypeId type_u21;//zhi:  May-17, 2004
TypeId type_u22;//zhi:  May-15, 2004
TypeId type_u23;//zhi:  May-17, 2004
TypeId type_u24;//zhi:  May-15, 2004
TypeId type_u25;//zhi:  May-17, 2004
TypeId type_u26;//zhi:  May-15, 2004
TypeId type_u27;//zhi:  May-17, 2004
TypeId type_u28;//zhi:  May-15, 2004
TypeId type_u29;//zhi:  May-17, 2004
TypeId type_u30;//zhi:  May-15, 2004
TypeId type_u31;//zhi:  May-17, 2004
TypeId type_s2;//zhi:  May-15, 2004
TypeId type_s3;//zhi:  May-17, 2004
TypeId type_s4;//zhi:  May-15, 2004
TypeId type_s5;//zhi:  May-17, 2004
TypeId type_s6;//zhi:  May-15, 2004
TypeId type_s7;//zhi:  May-17, 2004
TypeId type_s9;//zhi:  May-17, 2004
TypeId type_s10;//zhi:  May-15, 2004
TypeId type_s11;//zhi:  May-17, 2004
TypeId type_s12;//zhi:  May-15, 2004
TypeId type_s13;//zhi:  May-17, 2004
TypeId type_s14;//zhi:  May-15, 2004
TypeId type_s15;//zhi:  May-17, 2004
TypeId type_s17;//zhi:  May-17, 2004
TypeId type_s18;//zhi:  May-15, 2004
TypeId type_s19;//zhi:  May-17, 2004
TypeId type_s20;//zhi:  May-15, 2004
TypeId type_s21;//zhi:  May-17, 2004
TypeId type_s22;//zhi:  May-15, 2004
TypeId type_s23;//zhi:  May-17, 2004
TypeId type_s24;//zhi:  May-15, 2004
TypeId type_s25;//zhi:  May-17, 2004
TypeId type_s26;//zhi:  May-15, 2004
TypeId type_s27;//zhi:  May-17, 2004
TypeId type_s28;//zhi:  May-15, 2004
TypeId type_s29;//zhi:  May-17, 2004
TypeId type_s30;//zhi:  May-15, 2004
TypeId type_s31;//zhi:  May-17, 2004
TypeId type_p8;		// pointers
TypeId type_p9;
TypeId type_p10;
TypeId type_p11;
TypeId type_p12;
TypeId type_p13;
TypeId type_p14;
TypeId type_p15;
TypeId type_p16;		// pointers
TypeId type_p17;
TypeId type_p18;
TypeId type_p19;
TypeId type_p20;
TypeId type_p21;
TypeId type_p22;
TypeId type_p23;
TypeId type_p24;
TypeId type_p25;
TypeId type_p26;
TypeId type_p27;
TypeId type_p28;
TypeId type_p29;
TypeId type_p30;
TypeId type_p31;
TypeId type_p32;
TypeId type_p33;
TypeId type_p34;
TypeId type_p35;
TypeId type_p36;
TypeId type_p37;
TypeId type_p38;
TypeId type_p39;
TypeId type_p40;
TypeId type_p41;
TypeId type_p42;
TypeId type_p43;
TypeId type_p44;
TypeId type_p45;
TypeId type_p46;
TypeId type_p47;
TypeId type_p48;
TypeId type_p49;
TypeId type_p50;
TypeId type_p51;
TypeId type_p52;
TypeId type_p53;
TypeId type_p54;
TypeId type_p55;
TypeId type_p56;
TypeId type_p57;
TypeId type_p58;
TypeId type_p59;
TypeId type_p60;
TypeId type_p61;
TypeId type_p62;
TypeId type_p63;
TypeId type_p64;
TypeId type_p65;
TypeId type_p66;
TypeId type_p67;
TypeId type_p68;
TypeId type_p69;
TypeId type_p70;
TypeId type_p71;
TypeId type_p72;
TypeId type_p73;
TypeId type_p74;
TypeId type_p75;
TypeId type_p76;
TypeId type_p77;
TypeId type_p78;
TypeId type_p79;
TypeId type_p80;
TypeId type_p81;
TypeId type_p82;
TypeId type_p83;
TypeId type_p84;
TypeId type_p85;
TypeId type_p86;
TypeId type_p87;
TypeId type_p88;
TypeId type_p89;
TypeId type_p90;
TypeId type_p91;
TypeId type_p92;
TypeId type_p93;
TypeId type_p94;
TypeId type_p95;
TypeId type_p96;
TypeId type_p97;
TypeId type_p98;
TypeId type_p99;
TypeId type_p100;
TypeId type_p101;
TypeId type_p102;
TypeId type_p103;
TypeId type_p104;
TypeId type_p105;
TypeId type_p106;
TypeId type_p107;
TypeId type_p108;
TypeId type_p109;
TypeId type_p110;
TypeId type_p111;
TypeId type_p112;
TypeId type_p113;
TypeId type_p114;
TypeId type_p115;
TypeId type_p116;
TypeId type_p117;
TypeId type_p118;
TypeId type_p119;
TypeId type_p120;
TypeId type_p121;
TypeId type_p122;
TypeId type_p123;
TypeId type_p124;
TypeId type_p125;
TypeId type_p126;
TypeId type_p127;
TypeId type_p128;
TypeId type_p129;
TypeId type_p130;
TypeId type_p131;
TypeId type_p132;
TypeId type_p133;
TypeId type_p134;
TypeId type_p135;
TypeId type_p136;
TypeId type_p137;
TypeId type_p138;
TypeId type_p139;
TypeId type_p140;
TypeId type_p141;
TypeId type_p142;
TypeId type_p143;
TypeId type_p144;
TypeId type_p145;
TypeId type_p146;
TypeId type_p147;
TypeId type_p148;
TypeId type_p149;
TypeId type_p150;
TypeId type_p151;
TypeId type_p152;
TypeId type_p153;
TypeId type_p154;
TypeId type_p155;
TypeId type_p156;
TypeId type_p157;
TypeId type_p158;
TypeId type_p159;
TypeId type_p160;
TypeId type_p161;
TypeId type_p162;
TypeId type_p163;
TypeId type_p164;
TypeId type_p165;
TypeId type_p166;
TypeId type_p167;
TypeId type_p168;
TypeId type_p169;
TypeId type_p170;
TypeId type_p171;
TypeId type_p172;
TypeId type_p173;
TypeId type_p174;
TypeId type_p175;
TypeId type_p176;
TypeId type_p177;
TypeId type_p178;
TypeId type_p179;
TypeId type_p180;
TypeId type_p181;
TypeId type_p182;
TypeId type_p183;
TypeId type_p184;
TypeId type_p185;
TypeId type_p186;
TypeId type_p187;
TypeId type_p188;
TypeId type_p189;
TypeId type_p190;
TypeId type_p191;
TypeId type_p192;
TypeId type_p193;
TypeId type_p194;
TypeId type_p195;
TypeId type_p196;
TypeId type_p197;
TypeId type_p198;
TypeId type_p199;
TypeId type_p200;
TypeId type_p201;
TypeId type_p202;
TypeId type_p203;
TypeId type_p204;
TypeId type_p205;
TypeId type_p206;
TypeId type_p207;
TypeId type_p208;
TypeId type_p209;
TypeId type_p210;
TypeId type_p211;
TypeId type_p212;
TypeId type_p213;
TypeId type_p214;
TypeId type_p215;
TypeId type_p216;
TypeId type_p217;
TypeId type_p218;
TypeId type_p219;
TypeId type_p220;
TypeId type_p221;
TypeId type_p222;
TypeId type_p223;
TypeId type_p224;
TypeId type_p225;
TypeId type_p226;
TypeId type_p227;
TypeId type_p228;
TypeId type_p229;
TypeId type_p230;
TypeId type_p231;
TypeId type_p232;
TypeId type_p233;
TypeId type_p234;
TypeId type_p235;
TypeId type_p236;
TypeId type_p237;
TypeId type_p238;
TypeId type_p239;
TypeId type_p240;
TypeId type_p241;
TypeId type_p242;
TypeId type_p243;
TypeId type_p244;
TypeId type_p245;
TypeId type_p246;
TypeId type_p247;
TypeId type_p248;
TypeId type_p249;
TypeId type_p250;
TypeId type_p251;
TypeId type_p252;
TypeId type_p253;
TypeId type_p254;
TypeId type_p255;
TypeId type_p256;		// pointers
TypeId type_p312;		// pointers
TypeId type_p512;		// pointers
TypeId type_p300;		// pointers
TypeId type_p600;		// pointers
TypeId type_p352;		// pointers
TypeId type_p704;		// pointers
TypeId type_p1;
TypeId type_p3;
TypeId type_p5;
TypeId type_p6;
TypeId type_p7;


TypeId* predefined_types_addresses[] = {
  &type_v0,
  &type_s8,  &type_s16, &type_s32, &type_s64,
  &type_u8,  &type_u16, &type_u32, &type_u64,
  &type_f32, &type_f64, &type_f128,
  &type_p2,  &type_p4,&type_u1,//zhi: boolean May-12, 2004
  &type_u2,  &type_u3,  &type_u4,   &type_u5,  &type_u6,   &type_u7,   &type_u9, //zhi: May-15, 2004
  &type_u10, &type_u11, &type_u12,  &type_u13, &type_u14,  &type_u15,  &type_u17,//zhi: May-15, 2004
  &type_u18, &type_u19, &type_u20,  &type_u21, &type_u22,  &type_u23,            //zhi: May-15, 2004
  &type_u24, &type_u25, &type_u26,  &type_u27, &type_u28,  &type_u29,  &type_u30,   &type_u31,//zhi: May-15, 2004
  &type_s2,  &type_s3,  &type_s4,   &type_s5,  &type_s6,   &type_s7,   &type_s9,      //zhi: May-15, 2004

  &type_s10,
  &type_s11,
  &type_s12,
  &type_s13,
  &type_s14,
  &type_s15,
  &type_s17,     //zhi: May-15, 2004

  &type_s18, &type_s19, &type_s20,  &type_s21, &type_s22,  &type_s23,                 //zhi: May-15, 2004
  &type_s24, &type_s25, &type_s26,  &type_s27, &type_s28,  &type_s29,  &type_s30,    &type_s31,//zhi: May-15, 2004        
  &type_p8,

  &type_p9,
  &type_p10,
  &type_p11,
  &type_p12,
  &type_p13,
  &type_p14,
  &type_p15,
  &type_p16,

  &type_p17,
  &type_p18,
  &type_p19,
  &type_p20,
  &type_p21,
  &type_p22,
  &type_p23,
  &type_p24,
  &type_p25,
  &type_p26,
  &type_p27,
  &type_p28,
  &type_p29,
  &type_p30,
  &type_p31,
  &type_p32,
  &type_p33,
  &type_p34,
  &type_p35,
  &type_p36,
  &type_p37,
  &type_p38,
  &type_p39,
  &type_p40,
  &type_p41,
  &type_p42,
  &type_p43,
  &type_p44,
  &type_p45,
  &type_p46,
  &type_p47,
  &type_p48,
  &type_p49,
  &type_p50,
  &type_p51,
  &type_p52,
  &type_p53,
  &type_p54,
  &type_p55,
  &type_p56,
  &type_p57,
  &type_p58,
  &type_p59,
  &type_p60,
  &type_p61,
  &type_p62,
  &type_p63,
  &type_p64,
  &type_p65,
  &type_p66,
  &type_p67,
  &type_p68,
  &type_p69,
  &type_p70,
  &type_p71,
  &type_p72,
  &type_p73,
  &type_p74,
  &type_p75,
  &type_p76,
  &type_p77,
  &type_p78,
  &type_p79,
  &type_p80,
  &type_p81,
  &type_p82,
  &type_p83,
  &type_p84,
  &type_p85,
  &type_p86,
  &type_p87,
  &type_p88,
  &type_p89,
  &type_p90,
  &type_p91,
  &type_p92,
  &type_p93,
  &type_p94,
  &type_p95,
  &type_p96,
  &type_p97,
  &type_p98,
  &type_p99,
  &type_p100,
  &type_p101,
  &type_p102,
  &type_p103,
  &type_p104,
  &type_p105,
  &type_p106,
  &type_p107,
  &type_p108,
  &type_p109,
  &type_p110,
  &type_p111,
  &type_p112,
  &type_p113,
  &type_p114,
  &type_p115,
  &type_p116,
  &type_p117,
  &type_p118,
  &type_p119,
  &type_p120,
  &type_p121,
  &type_p122,
  &type_p123,
  &type_p124,
  &type_p125,
  &type_p126,
  &type_p127,
  &type_p128,
  &type_p129,
  &type_p130,
  &type_p131,
  &type_p132,
  &type_p133,
  &type_p134,
  &type_p135,
  &type_p136,
  &type_p137,
  &type_p138,
  &type_p139,
  &type_p140,
  &type_p141,
  &type_p142,
  &type_p143,
  &type_p144,
  &type_p145,
  &type_p146,
  &type_p147,
  &type_p148,
  &type_p149,
  &type_p150,
  &type_p151,
  &type_p152,
  &type_p153,
  &type_p154,
  &type_p155,
  &type_p156,
  &type_p157,
  &type_p158,
  &type_p159,
  &type_p160,
  &type_p161,
  &type_p162,
  &type_p163,
  &type_p164,
  &type_p165,
  &type_p166,
  &type_p167,
  &type_p168,
  &type_p169,
  &type_p170,
  &type_p171,
  &type_p172,
  &type_p173,
  &type_p174,
  &type_p175,
  &type_p176,
  &type_p177,
  &type_p178,
  &type_p179,
  &type_p180,
  &type_p181,
  &type_p182,
  &type_p183,
  &type_p184,
  &type_p185,
  &type_p186,
  &type_p187,
  &type_p188,
  &type_p189,
  &type_p190,
  &type_p191,
  &type_p192,
  &type_p193,
  &type_p194,
  &type_p195,
  &type_p196,
  &type_p197,
  &type_p198,
  &type_p199,
  &type_p200,
  &type_p201,
  &type_p202,
  &type_p203,
  &type_p204,
  &type_p205,
  &type_p206,
  &type_p207,
  &type_p208,
  &type_p209,
  &type_p210,
  &type_p211,
  &type_p212,
  &type_p213,
  &type_p214,
  &type_p215,
  &type_p216,
  &type_p217,
  &type_p218,
  &type_p219,
  &type_p220,
  &type_p221,
  &type_p222,
  &type_p223,
  &type_p224,
  &type_p225,
  &type_p226,
  &type_p227,
  &type_p228,
  &type_p229,
  &type_p230,
  &type_p231,
  &type_p232,
  &type_p233,
  &type_p234,
  &type_p235,
  &type_p236,
  &type_p237,
  &type_p238,
  &type_p239,
  &type_p240,
  &type_p241,
  &type_p242,
  &type_p243,
  &type_p244,
  &type_p245,
  &type_p246,
  &type_p247,
  &type_p248,
  &type_p249,
  &type_p250,
  &type_p251,
  &type_p252,
  &type_p253,
  &type_p254,
  &type_p255,    
  &type_p256,
  &type_p312,
  &type_p512,
  &type_p300,
  &type_p600,
  &type_p352,
  &type_p704,
  &type_p1,
  &type_p3,
  &type_p5,
  &type_p6,
  &type_p7
};

#define PREDEFINED_TYPES_SIZE \
  (int)(sizeof(predefined_types_addresses)/sizeof(TypeId*))


/*
 * Use the SUIF type builder to find or create machine-independent types in
 * the external symbol table.  Attach them as the "generic_types" note of
 * the file-set block.
 */
  void
attach_opi_predefined_types(FileSetBlock *the_file_set_block)
{
  claim(the_suif_env != NULL);
  TypeBuilder *tb =
    (TypeBuilder *)the_suif_env->
    get_object_factory(TypeBuilder::get_class_name());
  claim(tb != NULL);

  TypeId types[PREDEFINED_TYPES_SIZE];

  types[ 0] = tb->get_void_type();
  types[ 1] = tb->get_integer_type( 8,  8, true); // signed
  types[ 2] = tb->get_integer_type(16, 16, true);
  types[ 3] = tb->get_integer_type(32, 32, true);
  types[ 4] = tb->get_integer_type(64, 64, true);
  types[ 5] = tb->get_integer_type( 8,  8, false); // unsigned
  types[ 6] = tb->get_integer_type(16, 16, false);
  types[ 7] = tb->get_integer_type(32, 32, false);
  types[ 8] = tb->get_integer_type(64, 64, false);
  types[ 9] = tb->get_floating_point_type( 32,  32);
  types[10] = tb->get_floating_point_type( 64,  64);
  types[11] = tb->get_floating_point_type(128, 128);
  types[12] = tb->get_pointer_type(2, 2, types[0]);
  types[13] = tb->get_pointer_type(4, 4, types[0]);
  types[14] = tb->get_integer_type(1, 1, false);    //zhi: boolean May-12, 2004
  types[15] = tb->get_integer_type(2,2 , false);    //zhi:  May-15, 2004
  types[16] = tb->get_integer_type(3,3 , false);    //zhi:  May-15, 2004    
  types[17] = tb->get_integer_type(4, 4, false);    //zhi:  May-15, 2004
  types[18] = tb->get_integer_type(5, 5, false);    //zhi:  May-15, 2004    
  types[19] = tb->get_integer_type(6, 6, false);    //zhi:  May-15, 2004
  types[20] = tb->get_integer_type(7, 7, false);    //zhi:  May-15, 2004
  types[21] = tb->get_integer_type(9, 9, false);    //zhi:  May-15, 2004    
  types[22] = tb->get_integer_type(10, 10, false);    //zhi:  May-15, 2004
  types[23] = tb->get_integer_type(11, 11, false);    //zhi:  May-15, 2004    
  types[24] = tb->get_integer_type(12, 12, false);    //zhi:  May-15, 2004
  types[25] = tb->get_integer_type(13, 13, false);    //zhi:  May-15, 2004    
  types[26] = tb->get_integer_type(14, 14, false);    //zhi:  May-15, 2004
  types[27] = tb->get_integer_type(15, 15, false);    //zhi:  May-15, 2004
  types[28] = tb->get_integer_type(17, 17, false);    //zhi:  May-15, 2004
  types[29] = tb->get_integer_type(18, 18, false);    //zhi:  May-15, 2004
  types[30] = tb->get_integer_type(19, 19, false);    //zhi:  May-15, 2004
  types[31] = tb->get_integer_type(20, 20, false);    //zhi:  May-15, 2004
  types[32] = tb->get_integer_type(21, 21, false);    //zhi:  May-15, 2004
  types[33] = tb->get_integer_type(22, 22, false);    //zhi:  May-15, 2004
  types[34] = tb->get_integer_type(23, 23, false);    //zhi:  May-15, 2004
  types[35] = tb->get_integer_type(24, 24, false);    //zhi:  May-15, 2004
  types[36] = tb->get_integer_type(25, 25, false);    //zhi:  May-15, 2004
  types[37] = tb->get_integer_type(26, 26, false);    //zhi:  May-15, 2004
  types[38] = tb->get_integer_type(27, 27, false);    //zhi:  May-15, 2004
  types[39] = tb->get_integer_type(28, 28, false);    //zhi:  May-15, 2004
  types[40] = tb->get_integer_type(29, 29, false);    //zhi:  May-15, 2004
  types[41] = tb->get_integer_type(30, 30, false);    //zhi:  May-15, 2004
  types[42] = tb->get_integer_type(31, 31, false);    //zhi:  May-15, 2004
  types[43] = tb->get_integer_type(2, 2, true);    //zhi:  May-15, 2004
  types[44] = tb->get_integer_type(3, 3, true);    //zhi:  May-15, 2004    
  types[45] = tb->get_integer_type(4, 4, true);    //zhi:  May-15, 2004
  types[46] = tb->get_integer_type(5, 5, true);    //zhi:  May-15, 2004
  types[47] = tb->get_integer_type(6, 6, true);    //zhi:  May-15, 2004
  types[48] = tb->get_integer_type(7, 7, true);    //zhi:  May-15, 2004
  types[49] = tb->get_integer_type(9, 9, true);    //zhi:  May-15, 2004
  types[50] = tb->get_integer_type(10, 10, true);    //zhi:  May-15, 2004
  types[51] = tb->get_integer_type(11, 11, true);    //zhi:  May-15, 2004
  types[52] = tb->get_integer_type(12, 12, true);    //zhi:  May-15, 2004
  types[53] = tb->get_integer_type(13, 13, true);    //zhi:  May-15, 2004
  types[54] = tb->get_integer_type(14, 14, true);    //zhi:  May-15, 2004
  types[55] = tb->get_integer_type(15, 15, true);    //zhi:  May-15, 2004
  types[56] = tb->get_integer_type(17, 17, true);    //zhi:  May-15, 2004
  types[57] = tb->get_integer_type(18, 18, true);    //zhi:  May-15, 2004
  types[58] = tb->get_integer_type(19, 19, true);    //zhi:  May-15, 2004
  types[59] = tb->get_integer_type(20, 20, true);    //zhi:  May-15, 2004
  types[60] = tb->get_integer_type(21, 21, true);    //zhi:  May-15, 2004
  types[61] = tb->get_integer_type(22, 22, true);    //zhi:  May-15, 2004
  types[62] = tb->get_integer_type(23, 23, true);    //zhi:  May-15, 2004
  types[63] = tb->get_integer_type(24, 24, true);    //zhi:  May-15, 2004
  types[64] = tb->get_integer_type(25, 25, true);    //zhi:  May-15, 2004
  types[65] = tb->get_integer_type(26, 26, true);    //zhi:  May-15, 2004
  types[66] = tb->get_integer_type(27, 27, true);    //zhi:  May-15, 2004
  types[67] = tb->get_integer_type(28, 28, true);    //zhi:  May-15, 2004
  types[68] = tb->get_integer_type(29, 29, true);    //zhi:  May-15, 2004
  types[69] = tb->get_integer_type(30, 30, true);    //zhi:  May-15, 2004
  types[70] = tb->get_integer_type(31, 31, true);    //zhi:  May-15, 2004
  types[71] = tb->get_pointer_type(8, 8, types[0]);
  types[72] = tb->get_pointer_type(9, 9, types[0]);
  types[73] = tb->get_pointer_type(10, 10, types[0]);
  types[74] = tb->get_pointer_type(11, 11, types[0]);
  types[75] = tb->get_pointer_type(12, 12, types[0]);
  types[76] = tb->get_pointer_type(13, 13, types[0]);
  types[77] = tb->get_pointer_type(14, 14, types[0]);
  types[78] = tb->get_pointer_type(15, 15, types[0]);
  types[79] = tb->get_pointer_type(16, 16, types[0]);



  types[80] = tb->get_pointer_type(17, 17, types[0]);
  types[81] = tb->get_pointer_type(18, 18, types[0]);
  types[82] = tb->get_pointer_type(19, 19, types[0]);
  types[83] = tb->get_pointer_type(20, 20, types[0]);
  types[84] = tb->get_pointer_type(21, 21, types[0]);
  types[85] = tb->get_pointer_type(22, 22, types[0]);
  types[86] = tb->get_pointer_type(23, 23, types[0]);
  types[87] = tb->get_pointer_type(24, 24, types[0]);
  types[88] = tb->get_pointer_type(25, 25, types[0]);
  types[89] = tb->get_pointer_type(26, 26, types[0]);
  types[90] = tb->get_pointer_type(27, 27, types[0]);
  types[91] = tb->get_pointer_type(28, 28, types[0]);
  types[92] = tb->get_pointer_type(29, 29, types[0]);
  types[93] = tb->get_pointer_type(30, 30, types[0]);
  types[94] = tb->get_pointer_type(31, 31, types[0]);
  types[95] = tb->get_pointer_type(32, 32, types[0]);
  types[96] = tb->get_pointer_type(33, 33, types[0]);
  types[97] = tb->get_pointer_type(34, 34, types[0]);
  types[98] = tb->get_pointer_type(35, 35, types[0]);
  types[99] = tb->get_pointer_type(36, 36, types[0]);
  types[100] = tb->get_pointer_type(37, 37, types[0]);
  types[101] = tb->get_pointer_type(38, 38, types[0]);
  types[102] = tb->get_pointer_type(39, 39, types[0]);
  types[103] = tb->get_pointer_type(40, 40, types[0]);
  types[104] = tb->get_pointer_type(41, 41, types[0]);
  types[105] = tb->get_pointer_type(42, 42, types[0]);
  types[106] = tb->get_pointer_type(43, 43, types[0]);
  types[107] = tb->get_pointer_type(44, 44, types[0]);
  types[108] = tb->get_pointer_type(45, 45, types[0]);
  types[109] = tb->get_pointer_type(46, 46, types[0]);
  types[110] = tb->get_pointer_type(47, 47, types[0]);
  types[111] = tb->get_pointer_type(48, 48, types[0]);
  types[112] = tb->get_pointer_type(49, 49, types[0]);
  types[113] = tb->get_pointer_type(50, 50, types[0]);
  types[114] = tb->get_pointer_type(51, 51, types[0]);
  types[115] = tb->get_pointer_type(52, 52, types[0]);
  types[116] = tb->get_pointer_type(53, 53, types[0]);
  types[117] = tb->get_pointer_type(54, 54, types[0]);
  types[118] = tb->get_pointer_type(55, 55, types[0]);
  types[119] = tb->get_pointer_type(56, 56, types[0]);
  types[120] = tb->get_pointer_type(57, 57, types[0]);
  types[121] = tb->get_pointer_type(58, 58, types[0]);
  types[122] = tb->get_pointer_type(59, 59, types[0]);
  types[123] = tb->get_pointer_type(60, 60, types[0]);
  types[124] = tb->get_pointer_type(61, 61, types[0]);
  types[125] = tb->get_pointer_type(62, 62, types[0]);
  types[126] = tb->get_pointer_type(63, 63, types[0]);
  types[127] = tb->get_pointer_type(64, 64, types[0]);
  types[128] = tb->get_pointer_type(65, 65, types[0]);
  types[129] = tb->get_pointer_type(66, 66, types[0]);
  types[130] = tb->get_pointer_type(67, 67, types[0]);
  types[131] = tb->get_pointer_type(68, 68, types[0]);
  types[132] = tb->get_pointer_type(69, 69, types[0]);
  types[133] = tb->get_pointer_type(70, 70, types[0]);
  types[134] = tb->get_pointer_type(71, 71, types[0]);
  types[135] = tb->get_pointer_type(72, 72, types[0]);
  types[136] = tb->get_pointer_type(73, 73, types[0]);
  types[137] = tb->get_pointer_type(74, 74, types[0]);
  types[138] = tb->get_pointer_type(75, 75, types[0]);
  types[139] = tb->get_pointer_type(76, 76, types[0]);
  types[140] = tb->get_pointer_type(77, 77, types[0]);
  types[141] = tb->get_pointer_type(78, 78, types[0]);
  types[142] = tb->get_pointer_type(79, 79, types[0]);
  types[143] = tb->get_pointer_type(80, 80, types[0]);
  types[144] = tb->get_pointer_type(81, 81, types[0]);
  types[145] = tb->get_pointer_type(82, 82, types[0]);
  types[146] = tb->get_pointer_type(83, 83, types[0]);
  types[147] = tb->get_pointer_type(84, 84, types[0]);
  types[148] = tb->get_pointer_type(85, 85, types[0]);
  types[149] = tb->get_pointer_type(86, 86, types[0]);
  types[150] = tb->get_pointer_type(87, 87, types[0]);
  types[151] = tb->get_pointer_type(88, 88, types[0]);
  types[152] = tb->get_pointer_type(89, 89, types[0]);
  types[153] = tb->get_pointer_type(90, 90, types[0]);
  types[154] = tb->get_pointer_type(91, 91, types[0]);
  types[155] = tb->get_pointer_type(92, 92, types[0]);
  types[156] = tb->get_pointer_type(93, 93, types[0]);
  types[157] = tb->get_pointer_type(94, 94, types[0]);
  types[158] = tb->get_pointer_type(95, 95, types[0]);
  types[159] = tb->get_pointer_type(96, 96, types[0]);
  types[160] = tb->get_pointer_type(97, 97, types[0]);
  types[161] = tb->get_pointer_type(98, 98, types[0]);
  types[162] = tb->get_pointer_type(99, 99, types[0]);
  types[163] = tb->get_pointer_type(100, 100, types[0]);
  types[164] = tb->get_pointer_type(101, 101, types[0]);
  types[165] = tb->get_pointer_type(102, 102, types[0]);
  types[166] = tb->get_pointer_type(103, 103, types[0]);
  types[167] = tb->get_pointer_type(104, 104, types[0]);
  types[168] = tb->get_pointer_type(105, 105, types[0]);
  types[169] = tb->get_pointer_type(106, 106, types[0]);
  types[170] = tb->get_pointer_type(107, 107, types[0]);
  types[171] = tb->get_pointer_type(108, 108, types[0]);
  types[172] = tb->get_pointer_type(109, 109, types[0]);
  types[173] = tb->get_pointer_type(110, 110, types[0]);
  types[174] = tb->get_pointer_type(111, 111, types[0]);
  types[175] = tb->get_pointer_type(112, 112, types[0]);
  types[176] = tb->get_pointer_type(113, 113, types[0]);
  types[177] = tb->get_pointer_type(114, 114, types[0]);
  types[178] = tb->get_pointer_type(115, 115, types[0]);
  types[179] = tb->get_pointer_type(116, 116, types[0]);
  types[180] = tb->get_pointer_type(117, 117, types[0]);
  types[181] = tb->get_pointer_type(118, 118, types[0]);
  types[182] = tb->get_pointer_type(119, 119, types[0]);
  types[183] = tb->get_pointer_type(120, 120, types[0]);
  types[184] = tb->get_pointer_type(121, 121, types[0]);
  types[185] = tb->get_pointer_type(122, 122, types[0]);
  types[186] = tb->get_pointer_type(123, 123, types[0]);
  types[187] = tb->get_pointer_type(124, 124, types[0]);
  types[188] = tb->get_pointer_type(125, 125, types[0]);
  types[189] = tb->get_pointer_type(126, 126, types[0]);
  types[190] = tb->get_pointer_type(127, 127, types[0]);
  types[191] = tb->get_pointer_type(128, 128, types[0]);
  types[192] = tb->get_pointer_type(129, 129, types[0]);
  types[193] = tb->get_pointer_type(130, 130, types[0]);
  types[194] = tb->get_pointer_type(131, 131, types[0]);
  types[195] = tb->get_pointer_type(132, 132, types[0]);
  types[196] = tb->get_pointer_type(133, 133, types[0]);
  types[197] = tb->get_pointer_type(134, 134, types[0]);
  types[198] = tb->get_pointer_type(135, 135, types[0]);
  types[199] = tb->get_pointer_type(136, 136, types[0]);
  types[200] = tb->get_pointer_type(137, 137, types[0]);
  types[201] = tb->get_pointer_type(138, 138, types[0]);
  types[202] = tb->get_pointer_type(139, 139, types[0]);
  types[203] = tb->get_pointer_type(140, 140, types[0]);
  types[204] = tb->get_pointer_type(141, 141, types[0]);
  types[205] = tb->get_pointer_type(142, 142, types[0]);
  types[206] = tb->get_pointer_type(143, 143, types[0]);
  types[207] = tb->get_pointer_type(144, 144, types[0]);
  types[208] = tb->get_pointer_type(145, 145, types[0]);
  types[209] = tb->get_pointer_type(146, 146, types[0]);
  types[210] = tb->get_pointer_type(147, 147, types[0]);
  types[211] = tb->get_pointer_type(148, 148, types[0]);
  types[212] = tb->get_pointer_type(149, 149, types[0]);
  types[213] = tb->get_pointer_type(150, 150, types[0]);
  types[214] = tb->get_pointer_type(151, 151, types[0]);
  types[215] = tb->get_pointer_type(152, 152, types[0]);
  types[216] = tb->get_pointer_type(153, 153, types[0]);
  types[217] = tb->get_pointer_type(154, 154, types[0]);
  types[218] = tb->get_pointer_type(155, 155, types[0]);
  types[219] = tb->get_pointer_type(156, 156, types[0]);
  types[220] = tb->get_pointer_type(157, 157, types[0]);
  types[221] = tb->get_pointer_type(158, 158, types[0]);
  types[222] = tb->get_pointer_type(159, 159, types[0]);
  types[223] = tb->get_pointer_type(160, 160, types[0]);
  types[224] = tb->get_pointer_type(161, 161, types[0]);
  types[225] = tb->get_pointer_type(162, 162, types[0]);
  types[226] = tb->get_pointer_type(163, 163, types[0]);
  types[227] = tb->get_pointer_type(164, 164, types[0]);
  types[228] = tb->get_pointer_type(165, 165, types[0]);
  types[229] = tb->get_pointer_type(166, 166, types[0]);
  types[230] = tb->get_pointer_type(167, 167, types[0]);
  types[231] = tb->get_pointer_type(168, 168, types[0]);
  types[232] = tb->get_pointer_type(169, 169, types[0]);
  types[233] = tb->get_pointer_type(170, 170, types[0]);
  types[234] = tb->get_pointer_type(171, 171, types[0]);
  types[235] = tb->get_pointer_type(172, 172, types[0]);
  types[236] = tb->get_pointer_type(173, 173, types[0]);
  types[237] = tb->get_pointer_type(174, 174, types[0]);
  types[238] = tb->get_pointer_type(175, 175, types[0]);
  types[239] = tb->get_pointer_type(176, 176, types[0]);
  types[240] = tb->get_pointer_type(177, 177, types[0]);
  types[241] = tb->get_pointer_type(178, 178, types[0]);
  types[242] = tb->get_pointer_type(179, 179, types[0]);
  types[243] = tb->get_pointer_type(180, 180, types[0]);
  types[244] = tb->get_pointer_type(181, 181, types[0]);
  types[245] = tb->get_pointer_type(182, 182, types[0]);
  types[246] = tb->get_pointer_type(183, 183, types[0]);
  types[247] = tb->get_pointer_type(184, 184, types[0]);
  types[248] = tb->get_pointer_type(185, 185, types[0]);
  types[249] = tb->get_pointer_type(186, 186, types[0]);
  types[250] = tb->get_pointer_type(187, 187, types[0]);
  types[251] = tb->get_pointer_type(188, 188, types[0]);
  types[252] = tb->get_pointer_type(189, 189, types[0]);
  types[253] = tb->get_pointer_type(190, 190, types[0]);
  types[254] = tb->get_pointer_type(191, 191, types[0]);
  types[255] = tb->get_pointer_type(192, 192, types[0]);
  types[256] = tb->get_pointer_type(193, 193, types[0]);
  types[257] = tb->get_pointer_type(194, 194, types[0]);
  types[258] = tb->get_pointer_type(195, 195, types[0]);
  types[259] = tb->get_pointer_type(196, 196, types[0]);
  types[260] = tb->get_pointer_type(197, 197, types[0]);
  types[261] = tb->get_pointer_type(198, 198, types[0]);
  types[262] = tb->get_pointer_type(199, 199, types[0]);
  types[263] = tb->get_pointer_type(200, 200, types[0]);
  types[264] = tb->get_pointer_type(201, 201, types[0]);
  types[265] = tb->get_pointer_type(202, 202, types[0]);
  types[266] = tb->get_pointer_type(203, 203, types[0]);
  types[267] = tb->get_pointer_type(204, 204, types[0]);
  types[268] = tb->get_pointer_type(205, 205, types[0]);
  types[269] = tb->get_pointer_type(206, 206, types[0]);
  types[270] = tb->get_pointer_type(207, 207, types[0]);
  types[271] = tb->get_pointer_type(208, 208, types[0]);
  types[272] = tb->get_pointer_type(209, 209, types[0]);
  types[273] = tb->get_pointer_type(210, 210, types[0]);
  types[274] = tb->get_pointer_type(211, 211, types[0]);
  types[275] = tb->get_pointer_type(212, 212, types[0]);
  types[276] = tb->get_pointer_type(213, 213, types[0]);
  types[277] = tb->get_pointer_type(214, 214, types[0]);
  types[278] = tb->get_pointer_type(215, 215, types[0]);
  types[279] = tb->get_pointer_type(216, 216, types[0]);
  types[280] = tb->get_pointer_type(217, 217, types[0]);
  types[281] = tb->get_pointer_type(218, 218, types[0]);
  types[282] = tb->get_pointer_type(219, 219, types[0]);
  types[283] = tb->get_pointer_type(220, 220, types[0]);
  types[284] = tb->get_pointer_type(221, 221, types[0]);
  types[285] = tb->get_pointer_type(222, 222, types[0]);
  types[286] = tb->get_pointer_type(223, 223, types[0]);
  types[287] = tb->get_pointer_type(224, 224, types[0]);
  types[288] = tb->get_pointer_type(225, 225, types[0]);
  types[289] = tb->get_pointer_type(226, 226, types[0]);
  types[290] = tb->get_pointer_type(227, 227, types[0]);
  types[291] = tb->get_pointer_type(228, 228, types[0]);
  types[292] = tb->get_pointer_type(229, 229, types[0]);
  types[293] = tb->get_pointer_type(230, 230, types[0]);
  types[294] = tb->get_pointer_type(231, 231, types[0]);
  types[295] = tb->get_pointer_type(232, 232, types[0]);
  types[296] = tb->get_pointer_type(233, 233, types[0]);
  types[297] = tb->get_pointer_type(234, 234, types[0]);
  types[298] = tb->get_pointer_type(235, 235, types[0]);
  types[299] = tb->get_pointer_type(236, 236, types[0]);
  types[300] = tb->get_pointer_type(237, 237, types[0]);
  types[301] = tb->get_pointer_type(238, 238, types[0]);
  types[302] = tb->get_pointer_type(239, 239, types[0]);
  types[303] = tb->get_pointer_type(240, 240, types[0]);
  types[304] = tb->get_pointer_type(241, 241, types[0]);
  types[305] = tb->get_pointer_type(242, 242, types[0]);
  types[306] = tb->get_pointer_type(243, 243, types[0]);
  types[307] = tb->get_pointer_type(244, 244, types[0]);
  types[308] = tb->get_pointer_type(245, 245, types[0]);
  types[309] = tb->get_pointer_type(246, 246, types[0]);
  types[310] = tb->get_pointer_type(247, 247, types[0]);
  types[311] = tb->get_pointer_type(248, 248, types[0]);
  types[312] = tb->get_pointer_type(249, 249, types[0]);
  types[313] = tb->get_pointer_type(250, 250, types[0]);
  types[314] = tb->get_pointer_type(251, 251, types[0]);
  types[315] = tb->get_pointer_type(252, 252, types[0]);
  types[316] = tb->get_pointer_type(253, 253, types[0]);
  types[317] = tb->get_pointer_type(254, 254, types[0]);
  types[318] = tb->get_pointer_type(255, 255, types[0]); 
  types[319] = tb->get_pointer_type(256, 256, types[0]);
  types[320] = tb->get_pointer_type(312, 312, types[0]);
  types[321] = tb->get_pointer_type(512, 512, types[0]);
  types[322] = tb->get_pointer_type(300, 300, types[0]);
  types[323] = tb->get_pointer_type(600, 600, types[0]);
  types[324] = tb->get_pointer_type(352, 352, types[0]);
  types[325] = tb->get_pointer_type(704, 704, types[0]);

  types[326] = tb->get_pointer_type(1, 1, types[0]);
  types[327] = tb->get_pointer_type(3, 3, types[0]);
  types[328] = tb->get_pointer_type(5, 5, types[0]);
  types[329] = tb->get_pointer_type(6, 6, types[0]);
  types[330] = tb->get_pointer_type(7, 7, types[0]);

  ListNote<IrObject*> note;	// k_generic_types

  for (int i = 0; i < PREDEFINED_TYPES_SIZE; ++i) {
    claim(types[i], "Type builder failed");
    note.append((IrObject*)types[i]);
  }
  set_note(the_file_set_block, k_generic_types, note);
  set_opi_predefined_types(the_file_set_block);
}

// Initializations for machine-independent types
  void
set_opi_predefined_types(FileSetBlock* fsb)
{

  ListNote<IrObject*> note = get_note(fsb, k_generic_types);
  claim(!is_null(note), "Missing k_generic_types note on FileSetBlock");

  for (int i = 0; i < note.values_size(); ++i)
    *(predefined_types_addresses[i]) = to<Type>(note.get_value(i));
}

  TypeId
type_addr()
{
  return dynamic_cast<MachineContext*>(the_context)->type_addr();
}

  void
fprint(FILE *file, TypeId type)
{
  Type *ut = unqualify_type(type);

  if (is_kind_of<VoidType>(ut)) {
    fputs("void", file);
  } else if (is_kind_of<IntegerType>(ut)) {
    IntegerType *it = (IntegerType*)ut;
    fprintf(file, "%c%d", it->get_is_signed() ? 's' : 'u', get_bit_size(it));
  } else if (is_kind_of<PointerType>(ut)) {
    fprintf(file, "p%d", get_bit_size(ut));
  } else if (is_kind_of<BooleanType>(ut)) {
    fprintf(file, "b%d", get_bit_size(ut));
  } else if (is_kind_of<FloatingPointType>(ut)) {
    fprintf(file, "f%d", get_bit_size(ut));
  } else {
    fprintf(file, "unhandled_type:%lx", (unsigned long)type);
  }
}


TypeId  build_typeid_type(TYPEID_enum_type et, int bitsize)
{
    claim(the_suif_env != NULL);
    TypeBuilder *tb =
        (TypeBuilder *)the_suif_env->
        get_object_factory(TypeBuilder::get_class_name());
    claim(tb != NULL);


    TypeId ret_type;
    TypeId void_type = tb->get_void_type();

    switch( et ) {
        case TYPEID_NONE:
            claim(false, "Yo dipshit, why bother building a NONE type");
            break;

        case TYPEID_VOID:
            claim( bitsize == 0, "voidtype is bitsize of zero");
            ret_type = void_type;
            break;

        case TYPEID_SINT:
            ret_type = tb->get_integer_type(bitsize, bitsize, true); // signed
            break;

        case TYPEID_UINT:
            ret_type = tb->get_integer_type(bitsize, bitsize, false); // unsigned
            break;

        case TYPEID_FLOAT:
            ret_type = tb->get_floating_point_type(bitsize, bitsize);
            break;

        case TYPEID_PTR:
            ret_type = tb->get_qualified_type(tb->get_pointer_type(bitsize, bitsize, void_type));
            break;

            // FIXME: Boolean?  I don't know if it should be integer type
        case TYPEID_BOOLEAN:
            claim( bitsize == 1 || bitsize == 8, "boolean type is bitsize of 8 or 1");
            ret_type = tb->get_boolean_type(); 
            break;

        default:
            claim(false, "Unknown type");
    }

    return ret_type;
}

