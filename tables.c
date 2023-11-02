/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2000 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *  Copyright 2023 by
 *  Frenkel Smeijers
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *      Lookup tables.
 *      Do not try to look them up :-).
 *      In the order of appearance:
 *
 *      int finesine[10240]             - Sine lookup.
 *       Guess what, serves as cosine, too.
 *       Remarkable thing is, how to use BAMs with this?
 *
 *-----------------------------------------------------------------------------
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stddef.h>
#include "w_wad.h"
#include "tables.h"
#include "globdata.h"


static const uint16_t finesineTable_part_1[2048] =
{
    25,75,125,175,226,276,326,376,
    427,477,527,578,628,678,728,779,
    829,879,929,980,1030,1080,1130,1181,
    1231,1281,1331,1382,1432,1482,1532,1583,
    1633,1683,1733,1784,1834,1884,1934,1985,
    2035,2085,2135,2186,2236,2286,2336,2387,
    2437,2487,2537,2587,2638,2688,2738,2788,
    2839,2889,2939,2989,3039,3090,3140,3190,
    3240,3291,3341,3391,3441,3491,3541,3592,
    3642,3692,3742,3792,3843,3893,3943,3993,
    4043,4093,4144,4194,4244,4294,4344,4394,
    4445,4495,4545,4595,4645,4695,4745,4796,
    4846,4896,4946,4996,5046,5096,5146,5197,
    5247,5297,5347,5397,5447,5497,5547,5597,
    5647,5697,5748,5798,5848,5898,5948,5998,
    6048,6098,6148,6198,6248,6298,6348,6398,
    6448,6498,6548,6598,6648,6698,6748,6798,
    6848,6898,6948,6998,7048,7098,7148,7198,
    7248,7298,7348,7398,7448,7498,7548,7598,
    7648,7697,7747,7797,7847,7897,7947,7997,
    8047,8097,8147,8196,8246,8296,8346,8396,
    8446,8496,8545,8595,8645,8695,8745,8794,
    8844,8894,8944,8994,9043,9093,9143,9193,
    9243,9292,9342,9392,9442,9491,9541,9591,
    9640,9690,9740,9790,9839,9889,9939,9988,
    10038,10088,10137,10187,10237,10286,10336,10386,
    10435,10485,10534,10584,10634,10683,10733,10782,
    10832,10882,10931,10981,11030,11080,11129,11179,
    11228,11278,11327,11377,11426,11476,11525,11575,
    11624,11674,11723,11773,11822,11872,11921,11970,
    12020,12069,12119,12168,12218,12267,12316,12366,
    12415,12464,12514,12563,12612,12662,12711,12760,
    12810,12859,12908,12957,13007,13056,13105,13154,
    13204,13253,13302,13351,13401,13450,13499,13548,
    13597,13647,13696,13745,13794,13843,13892,13941,
    13990,14040,14089,14138,14187,14236,14285,14334,
    14383,14432,14481,14530,14579,14628,14677,14726,
    14775,14824,14873,14922,14971,15020,15069,15118,
    15167,15215,15264,15313,15362,15411,15460,15509,
    15557,15606,15655,15704,15753,15802,15850,15899,
    15948,15997,16045,16094,16143,16191,16240,16289,
    16338,16386,16435,16484,16532,16581,16629,16678,
    16727,16775,16824,16872,16921,16970,17018,17067,
    17115,17164,17212,17261,17309,17358,17406,17455,
    17503,17551,17600,17648,17697,17745,17793,17842,
    17890,17939,17987,18035,18084,18132,18180,18228,
    18277,18325,18373,18421,18470,18518,18566,18614,
    18663,18711,18759,18807,18855,18903,18951,19000,
    19048,19096,19144,19192,19240,19288,19336,19384,
    19432,19480,19528,19576,19624,19672,19720,19768,
    19816,19864,19912,19959,20007,20055,20103,20151,
    20199,20246,20294,20342,20390,20438,20485,20533,
    20581,20629,20676,20724,20772,20819,20867,20915,
    20962,21010,21057,21105,21153,21200,21248,21295,
    21343,21390,21438,21485,21533,21580,21628,21675,
    21723,21770,21817,21865,21912,21960,22007,22054,
    22102,22149,22196,22243,22291,22338,22385,22433,
    22480,22527,22574,22621,22668,22716,22763,22810,
    22857,22904,22951,22998,23045,23092,23139,23186,
    23233,23280,23327,23374,23421,23468,23515,23562,
    23609,23656,23703,23750,23796,23843,23890,23937,
    23984,24030,24077,24124,24171,24217,24264,24311,
    24357,24404,24451,24497,24544,24591,24637,24684,
    24730,24777,24823,24870,24916,24963,25009,25056,
    25102,25149,25195,25241,25288,25334,25381,25427,
    25473,25520,25566,25612,25658,25705,25751,25797,
    25843,25889,25936,25982,26028,26074,26120,26166,
    26212,26258,26304,26350,26396,26442,26488,26534,
    26580,26626,26672,26718,26764,26810,26856,26902,
    26947,26993,27039,27085,27131,27176,27222,27268,
    27313,27359,27405,27450,27496,27542,27587,27633,
    27678,27724,27770,27815,27861,27906,27952,27997,
    28042,28088,28133,28179,28224,28269,28315,28360,
    28405,28451,28496,28541,28586,28632,28677,28722,
    28767,28812,28858,28903,28948,28993,29038,29083,
    29128,29173,29218,29263,29308,29353,29398,29443,
    29488,29533,29577,29622,29667,29712,29757,29801,
    29846,29891,29936,29980,30025,30070,30114,30159,
    30204,30248,30293,30337,30382,30426,30471,30515,
    30560,30604,30649,30693,30738,30782,30826,30871,
    30915,30959,31004,31048,31092,31136,31181,31225,
    31269,31313,31357,31402,31446,31490,31534,31578,
    31622,31666,31710,31754,31798,31842,31886,31930,
    31974,32017,32061,32105,32149,32193,32236,32280,
    32324,32368,32411,32455,32499,32542,32586,32630,
    32673,32717,32760,32804,32847,32891,32934,32978,
    33021,33065,33108,33151,33195,33238,33281,33325,
    33368,33411,33454,33498,33541,33584,33627,33670,
    33713,33756,33799,33843,33886,33929,33972,34015,
    34057,34100,34143,34186,34229,34272,34315,34358,
    34400,34443,34486,34529,34571,34614,34657,34699,
    34742,34785,34827,34870,34912,34955,34997,35040,
    35082,35125,35167,35210,35252,35294,35337,35379,
    35421,35464,35506,35548,35590,35633,35675,35717,
    35759,35801,35843,35885,35927,35969,36011,36053,
    36095,36137,36179,36221,36263,36305,36347,36388,
    36430,36472,36514,36555,36597,36639,36681,36722,
    36764,36805,36847,36889,36930,36972,37013,37055,
    37096,37137,37179,37220,37262,37303,37344,37386,
    37427,37468,37509,37551,37592,37633,37674,37715,
    37756,37797,37838,37879,37920,37961,38002,38043,
    38084,38125,38166,38207,38248,38288,38329,38370,
    38411,38451,38492,38533,38573,38614,38655,38695,
    38736,38776,38817,38857,38898,38938,38979,39019,
    39059,39100,39140,39180,39221,39261,39301,39341,
    39382,39422,39462,39502,39542,39582,39622,39662,
    39702,39742,39782,39822,39862,39902,39942,39982,
    40021,40061,40101,40141,40180,40220,40260,40300,
    40339,40379,40418,40458,40497,40537,40576,40616,
    40655,40695,40734,40773,40813,40852,40891,40931,
    40970,41009,41048,41087,41127,41166,41205,41244,
    41283,41322,41361,41400,41439,41478,41517,41556,
    41595,41633,41672,41711,41750,41788,41827,41866,
    41904,41943,41982,42020,42059,42097,42136,42174,
    42213,42251,42290,42328,42366,42405,42443,42481,
    42520,42558,42596,42634,42672,42711,42749,42787,
    42825,42863,42901,42939,42977,43015,43053,43091,
    43128,43166,43204,43242,43280,43317,43355,43393,
    43430,43468,43506,43543,43581,43618,43656,43693,
    43731,43768,43806,43843,43880,43918,43955,43992,
    44029,44067,44104,44141,44178,44215,44252,44289,
    44326,44363,44400,44437,44474,44511,44548,44585,
    44622,44659,44695,44732,44769,44806,44842,44879,
    44915,44952,44989,45025,45062,45098,45135,45171,
    45207,45244,45280,45316,45353,45389,45425,45462,
    45498,45534,45570,45606,45642,45678,45714,45750,
    45786,45822,45858,45894,45930,45966,46002,46037,
    46073,46109,46145,46180,46216,46252,46287,46323,
    46358,46394,46429,46465,46500,46536,46571,46606,
    46642,46677,46712,46747,46783,46818,46853,46888,
    46923,46958,46993,47028,47063,47098,47133,47168,
    47203,47238,47273,47308,47342,47377,47412,47446,
    47481,47516,47550,47585,47619,47654,47688,47723,
    47757,47792,47826,47860,47895,47929,47963,47998,
    48032,48066,48100,48134,48168,48202,48237,48271,
    48305,48338,48372,48406,48440,48474,48508,48542,
    48575,48609,48643,48676,48710,48744,48777,48811,
    48844,48878,48911,48945,48978,49012,49045,49078,
    49112,49145,49178,49211,49244,49278,49311,49344,
    49377,49410,49443,49476,49509,49542,49575,49608,
    49640,49673,49706,49739,49771,49804,49837,49869,
    49902,49935,49967,50000,50032,50065,50097,50129,
    50162,50194,50226,50259,50291,50323,50355,50387,
    50420,50452,50484,50516,50548,50580,50612,50644,
    50675,50707,50739,50771,50803,50834,50866,50898,
    50929,50961,50993,51024,51056,51087,51119,51150,
    51182,51213,51244,51276,51307,51338,51369,51401,
    51432,51463,51494,51525,51556,51587,51618,51649,
    51680,51711,51742,51773,51803,51834,51865,51896,
    51926,51957,51988,52018,52049,52079,52110,52140,
    52171,52201,52231,52262,52292,52322,52353,52383,
    52413,52443,52473,52503,52534,52564,52594,52624,
    52653,52683,52713,52743,52773,52803,52832,52862,
    52892,52922,52951,52981,53010,53040,53069,53099,
    53128,53158,53187,53216,53246,53275,53304,53334,
    53363,53392,53421,53450,53479,53508,53537,53566,
    53595,53624,53653,53682,53711,53739,53768,53797,
    53826,53854,53883,53911,53940,53969,53997,54026,
    54054,54082,54111,54139,54167,54196,54224,54252,
    54280,54308,54337,54365,54393,54421,54449,54477,
    54505,54533,54560,54588,54616,54644,54672,54699,
    54727,54755,54782,54810,54837,54865,54892,54920,
    54947,54974,55002,55029,55056,55084,55111,55138,
    55165,55192,55219,55246,55274,55300,55327,55354,
    55381,55408,55435,55462,55489,55515,55542,55569,
    55595,55622,55648,55675,55701,55728,55754,55781,
    55807,55833,55860,55886,55912,55938,55965,55991,
    56017,56043,56069,56095,56121,56147,56173,56199,
    56225,56250,56276,56302,56328,56353,56379,56404,
    56430,56456,56481,56507,56532,56557,56583,56608,
    56633,56659,56684,56709,56734,56760,56785,56810,
    56835,56860,56885,56910,56935,56959,56984,57009,
    57034,57059,57083,57108,57133,57157,57182,57206,
    57231,57255,57280,57304,57329,57353,57377,57402,
    57426,57450,57474,57498,57522,57546,57570,57594,
    57618,57642,57666,57690,57714,57738,57762,57785,
    57809,57833,57856,57880,57903,57927,57950,57974,
    57997,58021,58044,58067,58091,58114,58137,58160,
    58183,58207,58230,58253,58276,58299,58322,58345,
    58367,58390,58413,58436,58459,58481,58504,58527,
    58549,58572,58594,58617,58639,58662,58684,58706,
    58729,58751,58773,58795,58818,58840,58862,58884,
    58906,58928,58950,58972,58994,59016,59038,59059,
    59081,59103,59125,59146,59168,59190,59211,59233,
    59254,59276,59297,59318,59340,59361,59382,59404,
    59425,59446,59467,59488,59509,59530,59551,59572,
    59593,59614,59635,59656,59677,59697,59718,59739,
    59759,59780,59801,59821,59842,59862,59883,59903,
    59923,59944,59964,59984,60004,60025,60045,60065,
    60085,60105,60125,60145,60165,60185,60205,60225,
    60244,60264,60284,60304,60323,60343,60363,60382,
    60402,60421,60441,60460,60479,60499,60518,60537,
    60556,60576,60595,60614,60633,60652,60671,60690,
    60709,60728,60747,60766,60785,60803,60822,60841,
    60859,60878,60897,60915,60934,60952,60971,60989,
    61007,61026,61044,61062,61081,61099,61117,61135,
    61153,61171,61189,61207,61225,61243,61261,61279,
    61297,61314,61332,61350,61367,61385,61403,61420,
    61438,61455,61473,61490,61507,61525,61542,61559,
    61577,61594,61611,61628,61645,61662,61679,61696,
    61713,61730,61747,61764,61780,61797,61814,61831,
    61847,61864,61880,61897,61913,61930,61946,61963,
    61979,61995,62012,62028,62044,62060,62076,62092,
    62108,62125,62141,62156,62172,62188,62204,62220,
    62236,62251,62267,62283,62298,62314,62329,62345,
    62360,62376,62391,62407,62422,62437,62453,62468,
    62483,62498,62513,62528,62543,62558,62573,62588,
    62603,62618,62633,62648,62662,62677,62692,62706,
    62721,62735,62750,62764,62779,62793,62808,62822,
    62836,62850,62865,62879,62893,62907,62921,62935,
    62949,62963,62977,62991,63005,63019,63032,63046,
    63060,63074,63087,63101,63114,63128,63141,63155,
    63168,63182,63195,63208,63221,63235,63248,63261,
    63274,63287,63300,63313,63326,63339,63352,63365,
    63378,63390,63403,63416,63429,63441,63454,63466,
    63479,63491,63504,63516,63528,63541,63553,63565,
    63578,63590,63602,63614,63626,63638,63650,63662,
    63674,63686,63698,63709,63721,63733,63745,63756,
    63768,63779,63791,63803,63814,63825,63837,63848,
    63859,63871,63882,63893,63904,63915,63927,63938,
    63949,63960,63971,63981,63992,64003,64014,64025,
    64035,64046,64057,64067,64078,64088,64099,64109,
    64120,64130,64140,64151,64161,64171,64181,64192,
    64202,64212,64222,64232,64242,64252,64261,64271,
    64281,64291,64301,64310,64320,64330,64339,64349,
    64358,64368,64377,64387,64396,64405,64414,64424,
    64433,64442,64451,64460,64469,64478,64487,64496,
    64505,64514,64523,64532,64540,64549,64558,64566,
    64575,64584,64592,64601,64609,64617,64626,64634,
    64642,64651,64659,64667,64675,64683,64691,64699,
    64707,64715,64723,64731,64739,64747,64754,64762,
    64770,64777,64785,64793,64800,64808,64815,64822,
    64830,64837,64844,64852,64859,64866,64873,64880,
    64887,64895,64902,64908,64915,64922,64929,64936,
    64943,64949,64956,64963,64969,64976,64982,64989,
    64995,65002,65008,65015,65021,65027,65033,65040,
    65046,65052,65058,65064,65070,65076,65082,65088,
    65094,65099,65105,65111,65117,65122,65128,65133,
    65139,65144,65150,65155,65161,65166,65171,65177,
    65182,65187,65192,65197,65202,65207,65212,65217,
    65222,65227,65232,65237,65242,65246,65251,65256,
    65260,65265,65270,65274,65279,65283,65287,65292,
    65296,65300,65305,65309,65313,65317,65321,65325,
    65329,65333,65337,65341,65345,65349,65352,65356,
    65360,65363,65367,65371,65374,65378,65381,65385,
    65388,65391,65395,65398,65401,65404,65408,65411,
    65414,65417,65420,65423,65426,65429,65431,65434,
    65437,65440,65442,65445,65448,65450,65453,65455,
    65458,65460,65463,65465,65467,65470,65472,65474,
    65476,65478,65480,65482,65484,65486,65488,65490,
    65492,65494,65496,65497,65499,65501,65502,65504,
    65505,65507,65508,65510,65511,65513,65514,65515,
    65516,65518,65519,65520,65521,65522,65523,65524,
    65525,65526,65527,65527,65528,65529,65530,65530,
    65531,65531,65532,65532,65533,65533,65534,65534,
    65534,65535,65535,65535,65535,65535,65535,65535
};


#define finesine_part_1(a) finesineTable_part_1[a]

static uint16_t finesine_part_2(int16_t x)
{
	x = 4095 - x;
	switch (x) {
		case  273: return 13646;
		case  771: return 36556;
		case  883: return 41088;
		case 1080: return 48304;
		case 1827: return 64600;
		default: return finesineTable_part_1[x];
	}
}

static fixed_t finesine_part_3(int16_t x)
{
	x -= 4096;
	switch (x) {
		case   51: return  -2588;
		case  863: return -40299;
		case 1078: return -48236;
		case 1080: return -48304;
		default: return 0xffff0000 | -finesineTable_part_1[x];
	}
}

static fixed_t finesine_part_4(int16_t x)
{
	x = 8191 - x;
	switch (x) {
		case   51: return  -2588;
		case  114: return  -5747;
		case  244: return -12217;
		case  455: return -22432;
		case  771: return -36556;
		case  795: return -37550;
		case  863: return -40299;
		case 1021: return -46251;
		case 1051: return -47307;
		case 1469: return -59189;
		default: return 0xffff0000 | -finesineTable_part_1[x];
	}
}


#define finecosine_part_1(a) finesine_part_2(a + (FINEANGLES / 4))
#define finecosine_part_2(a) finesine_part_3(a + (FINEANGLES / 4))
#define finecosine_part_3(a) finesine_part_4(a + (FINEANGLES / 4))

static uint16_t finecosine_part_4(int16_t x)
{
	x -= 6144;
	switch (x) {
		case   70: return  3542;
		case  114: return  5747;
		case  455: return 22432;
		case  629: return 30427;
		case  631: return 30516;
		case  771: return 36556;
		case  863: return 40299;
		case 1067: return 47861;
		case 1133: return 50064;
		case 1259: return 53912;
		case 1273: return 54309;
		case 1827: return 64600;
		default: return finesineTable_part_1[x];
	}
}

fixed_t finesine(int16_t x)
{
	if (x < 2048) {			//    0 <= x < 2048
		return finesine_part_1(x);
	} else if (x < 4096) {	// 2048 <= x < 4096
		return finesine_part_2(x);
	} else if (x < 6144) {	// 4096 <= x < 6144
		return finesine_part_3(x);
	} else {				// 6144 <= x < 8192
		return finesine_part_4(x);
	}
}


fixed_t finecosine(int16_t x)
{
	if (x < 2048) {			//    0 <= x < 2048
		return finecosine_part_1(x);
	} else if (x < 4096) {	// 2048 <= x < 4096
		return finecosine_part_2(x);
	} else if (x < 6144) {	// 4096 <= x < 6144
		return finecosine_part_3(x);
	} else {				// 6144 <= x < 8192
		return finecosine_part_4(x);
	}
}


static const uint16_t xtoviewangleTable[SCREENWIDTH + 1] =
{
	0x2008, 0x1FB0, 0x1F50, 0x1EF8, 0x1EA0, 0x1E40, 0x1DE0, 0x1D80,
	0x1D20, 0x1CB8, 0x1C50, 0x1BE8, 0x1B80, 0x1B18, 0x1AA8, 0x1A38,
	0x19C8, 0x1958, 0x18E8, 0x1870, 0x17F8, 0x1780, 0x1708, 0x1688,
	0x1608, 0x1588, 0x1508, 0x1480, 0x13F8, 0x1370, 0x12E8, 0x1258,
	0x11D0, 0x1140, 0x10B0, 0x1018, 0x0F80, 0x0EF0, 0x0E58, 0x0DB8,
	0x0D20, 0x0C80, 0x0BE0, 0x0B40, 0x0AA0, 0x0A00, 0x0958, 0x08B0,
	0x0810, 0x0768, 0x06C0, 0x0610, 0x0568, 0x04C0, 0x0410, 0x0360,
	0x02B8, 0x0208, 0x0158, 0x00B0, 0x0000, 0xFF50, 0xFEA8, 0xFDF8,
	0xFD48, 0xFCA0, 0xFBF0, 0xFB40, 0xFA98, 0xF9F0, 0xF940, 0xF898,
	0xF7F0, 0xF750, 0xF6A8, 0xF600, 0xF560, 0xF4C0, 0xF420, 0xF380,
	0xF2E0, 0xF248, 0xF1A8, 0xF110, 0xF080, 0xEFE8, 0xEF50, 0xEEC0,
	0xEE30, 0xEDA8, 0xED18, 0xEC90, 0xEC08, 0xEB80, 0xEAF8, 0xEA78,
	0xE9F8, 0xE978, 0xE8F8, 0xE880, 0xE808, 0xE790, 0xE718, 0xE6A8,
	0xE638, 0xE5C8, 0xE558, 0xE4E8, 0xE480, 0xE418, 0xE3B0, 0xE348,
	0xE2E0, 0xE280, 0xE220, 0xE1C0, 0xE160, 0xE108, 0xE0B0, 0xE050,
	0xC000
};

angle_t xtoviewangle(int16_t x)
{
	return ((uint32_t)xtoviewangleTable[x]) << FRACBITS;
}
