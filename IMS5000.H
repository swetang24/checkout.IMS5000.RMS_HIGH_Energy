#ifndef _IMS5000_H
#define _IMS5000_H

/******************************************************************************
    NAME : IMS5000.H
    DESC : Definitions for IMS5000
******************************************************************************/
#include <im5000\im5000.h>
#include <imslib32\imslib32.h>
//#include <jetdbo32\jetdbo32.h>

/******************************************************************************
    PUBLIC C and RC DEFINES
******************************************************************************/
// Error stringtable IDs
#define IDE_INTERNAL       0500 // Internal errors
#define IDE_VALIDATOR      0501 // Validator plugin errors
#define IDE_IMSRUNNING     1000
#define IDE_IMSINIT        1001
#define IDE_IMSINSTINIT    1002
#define IDE_BADIMAPI       1003
#define IDE_CREATEIM       1004
#define IDE_BADIMSLIB1API  1005
#define IDE_CREATEFILE     1006
#define IDE_WRITEFILE      1007
#define IDE_SHELLFAILED    1008
#define IDE_CDPROGS        1009
#define IDE_FILEDELFAILED  1010
#define IDE_OPENIMP        1011
#define IDE_FILETYPE       1012
#define IDE_NOMEM          1013
#define IDE_READFILE       1014
#define IDE_CREATEIMP      1015
#define IDE_INVALIDIMP     1016
#define IDE_FNF            1017
#define IDE_IMPMAXVARS     1018
#define IDE_IMPMAXMARKS    1019
#define IDE_BADVNAME       1020
#define IDE_BADVWIDTH      1021
#define IDE_VWIDTHTOOSMALL 1022
#define IDE_VMAXTOOSMALL   1023
#define IDE_VBADSTEP       1024
#define IDE_VMAXOFFSTEP    1025
#define IDE_BADVCHAR       1026
#define IDE_VOFFSTEP       1027
#define IDE_VTOOSMALL      1028
#define IDE_VTOOLARGE      1029
#define IDE_VCOUNTTOOLARGE 1030
#define IDE_SVALDONE       1031
#define IDE_VNEARLYEND     1032
#define IDE_VNFULL         1033
#define IDE_VARNOTFOUND    1034
#define IDE_BADIMPORIGIN   1035
#define IDE_BADX           1036
#define IDE_BADY           1037
#define IDE_BADCW          1038
#define IDE_BADMULTISTROKE 1039
#define IDE_BADTEXT        1040
#define IDE_BADXRES        1041
#define IDE_BADYRES        1042
#define IDE_BADBMFN        1043
#define IDE_BADCH          1044
#define IDE_BADZ           1045
#define IDE_SYSVARNOTFOUND 1046
#define IDE_USERIDEXISTS   1047
#define IDE_USERIDUNKNOWN  1048
#define IDE_USERACODEBAD   1049
#define IDE_USERLEVELBAD   1050
#define IDE_USERIDBAD      1051
#define IDE_USERNAMEBAD    1052
#define IDE_PASSWORDBAD    1053
#define IDE_OPENUSERFILE   1054
#define IDE_READUSERFILE   1055
#define IDE_BADFONT        1056
#define IDE_FINDIJ         1057
#define IDE_HANOTFOUND     1058
#define IDE_NOTIJFONT      1059
#define IDE_OPENFILE       1060
#define IDE_OPENIMF        1061
#define IDE_MAXFONTS       1062
#define IDE_BADCWR         1063
#define IDE_BADNMP         1064
#define IDE_BADMPS         1065
#define IDE_BADFMPN        1066
#define IDE_BADMPNI        1067
#define IDE_BADYOFS        1068
#define IDE_BADPMC         1069
#define IDE_CREATEIMF      1070
#define IDE_NOIMF          1071
#define IDE_BADWIDTH       1072
#define IDE_BADHEIGHT      1073
#define IDE_BADQZONE       1074
#define IDE_BADERODE       1075
#define IDE_NOCANDDERUN    1076
#define IDE_BADLOCATION    1077
#define IDE_BADIMFEXTENT   1078
#define IDE_BADIMFROTEXTENT   1130 ///----------10/05/2016
#define IDE_NOIMFNESTS     1079
#define IDE_BADHRPICT      1080
#define IDE_BADUDHANAME    1081
#define IDE_IMPMAXUDHAS    1082
#define IDE_IMPMAXINCUDHAS 1083
#define IDE_BADDAYSOFFSET  1084
#define IDE_BADHOURSOFFSET 1085
#define IDE_BADMINUTESOFFSET 1086
#define IDE_BADSECONDSOFFSET 1087
#define IDE_DESCNOTFOUND     1088
#define IDE_GRIDTOOBIG       1089
#define IDE_BADREP           1090
#define IDE_BADIMFORIGIN     1091
#define IDE_BADINSPECTPREP   1092
#define IDE_FAILSTOP         1093
#define IDE_BADDMCOLS        1094
#define IDE_BADDMROWS        1095
#define IDE_BADXQZ           1096
#define IDE_BADYQZ           1097
#define IDE_BADXSIZE         1098
#define IDE_BADYSIZE         1099
#define IDE_BVALDONE         1100
#define IDE_BADBASE          1101
#define IDE_ALLMARKSBLANK    1102
#define IDE_BADCONFIG        1103
#define IDE_ONPARTDONEFAILED 1104
#define IDE_BADROT           1105
#define IDE_BADDEG           1129    //deg sam
#define IDE_BADSLANT         1106
#define IDE_BADREADPREP      1107
#define IDE_BADVALIDATOR     1108
#define IDE_AELENDELTA       1109
#define IDE_AEROTDELTA       1110
#define IDE_AE0DELTA         1111
#define IDE_NOCALS           1112
#define IDE_BADFIELD         1113
#define IDE_BADFRAME         1114
#define IDE_BADVEL           1115
#define IDE_NOCURE           1116
#define IDE_BADBFI           1117
#define IDE_COMPBUF          1118
#define IDE_BADUPC           1119
#define IDE_BADC2OF5         1120
#define IDE_BADC39           1121
#define IDE_BADC93           1122
#define IDE_BADDEVBC         1123
#define IDE_BADXDOTS         1124
#define IDE_BADYDOTS         1125
#define IDE_BADALIGNPREP     1126
#define IDE_BADLOCATEPREP    1127
#define IDE_BADREADER        1128

//#define IDE_BADALIGNPREP  
//#define IDE_BADLOCATEPREP 

//#define IDE_BADIMFEXT      1077
//#define IDE_BADIMFOORG     1078
//#define IDE_BADIMFOEXT     1079
//#define IDE_BADIMFSW       1080

// Warning stringtable IDs
#define IDW_NOIJ          2000
#define IDW_NOFONTS       2001
#define IDW_HASBMARKS     2002
#define IDW_BADRF         2003
#define IDW_BADBMPFMT     2004
#define IDW_CDEVBMPFMT    2005
#define IDW_TOOMANYBMPS   2006


// Other stringtable IDs
#define IDS_BLANKVALS      2500
#define IDS_UNIQUEVALS     2501
#define IDS_UNIQUEHIST     2502
#define IDS_UNIQUEBOTH     2503
#define IDS_UNIQUEVALSSTOP 2504
#define IDS_UNIQUEHISTSTOP 2505
#define IDS_UNIQUEBOTHSTOP 2506

// Special Control IDs
#define IDC_MODE           4000
#define IDC_IM             4001

// Dialog Control IDs
#define IDC_VERSION        4100
#define IDC_EDIT           4101
#define IDC_NAME           4102
#define IDC_LISTBOX        4103
#define IDC_WIDTH          4104
#define IDC_DIGITS         4105
#define IDC_LOWERS         4106
#define IDC_SPACE          4107
#define IDC_UPPERS         4108
#define IDC_OTHERS         4109
#define IDC_ZEROS          4110
#define IDC_MIN            4111
#define IDC_MAX            4112
#define IDC_LOOP           4113
#define IDC_STEP           4114
#define IDC_DOWN           4115
#define IDC_REPEATS        4116
#define IDC_X              4117
#define IDC_Y              4118
#define IDC_FONTS          4119
#define IDC_WEXT           4120
#define IDC_MULTISTROKE    4121
#define IDC_FN             4122
#define IDC_NEG            4123
#define IDC_XRES           4124
#define IDC_YRES           4125
#define IDC_LICENSEE       4126
#define IDC_SN             4127
#define IDC_ADDUSER        4128
#define IDC_EDITUSER       4129
#define IDC_DELUSER        4130
#define IDC_USERID         4131
#define IDC_USERNAME       4132
#define IDC_PASSWORD       4133
#define IDC_USERLEVEL0     4134
#define IDC_USERLEVEL1     4135
#define IDC_USERLEVEL2     4136
#define IDC_USERLEVEL3     4137
#define IDC_USERLEVEL4     4138
#define IDC_JUST           4139
#define IDC_XFLIP          4140
#define IDC_YFLIP          4141
#define IDC_MPS            4142
#define IDC_NMP            4143
#define IDC_FMPN           4144
#define IDC_MPNI           4145
#define IDC_FIXTUREFILE    4146          //-------------Linghai-11-6-08-------------
#define IDC_BITMAPFILE	   4147          //-------------Linghai-11-10-08------
#define IDC_HINTDMM        4148          //-------------Linghai-11-17-08-------
#define IDC_IMAGE          4149          //--------------Linghai-11-13-08----
#define IDC_SEARCHFILE     4150
#define IDC_PRE			   4151         //--------------Linghai-7-9-09---
#define IDC_RS			   4152         //--------------Linghai-7-10-09--
#define IDC_GS		       4153         //--------------Linghai-7-10-09-
#define IDC_EDITX          4154
#define IDC_EDITY          4155
#define IDC_EDITDIA        4156
#define IDC_EDITHIG        4157
//#define IDC_EDITL          4156
//#define IDC_EDITW          4157
//#define IDC_EDITH          4158
//#define IDC_BROWSELINK     4159
//#define IDC_EDIT1LINK      4160
//#define IDC_LINKDISPLAY	   4161
#define IDC_HEX            4162
#define IDC_CHTEXT         4163
#define IDC_RHTEXT         4164
#define IDC_D              4565		//--sam	10/13/16
#define IDC_DLBL			4566	//--sam	10/14/16
//#define IDC_EDITW			4567	//--sam	10/14/16
#define IDC_EDITZ			4568	//--sam	10/14/16

// WMTRX
#define IDC_MTRX_SRC_XS 4500
#define IDC_MTRX_SRC_YS 4501
#define IDC_MTRX_SUB_XO 4502
#define IDC_MTRX_SUB_YO 4503
#define IDC_MTRX_SUB_XS 4504
#define IDC_MTRX_SUB_YS 4505
#define IDC_MTRX_CEL_XS 4506
#define IDC_MTRX_CEL_YS 4507
#define IDC_MTRX_DOT_XS 4508
#define IDC_MTRX_DOT_YS 4519
#define IDC_MTRX_MAT_XO 4520
#define IDC_MTRX_MAT_YO 4521
#define IDC_MTRX_CH_XO  4522
#define IDC_MTRX_CH_YO  4523
#define IDC_MTRX_RH_XO  4524
#define IDC_MTRX_RH_YO  4525
#define IDC_MTRX_RHR    4526
#define IDC_MTRX_CHB    4527
#define IDC_MTRX_ROUND  4528


#define IDC_BEGIN		   7000         //--------------Linghai-7-10-09-
#define IDC_UIDFORMAT      7001
#define IDC_END			   7002         //--------------Linghai-7-13-09-
#define IDC_ENTERID		   7003
#define IDC_PIN	           7004
#define IDC_PARTN		   7005
#define IDC_LOTBN          7006

#define IDC_ENTERIDv	   7007
#define IDC_PINv	       7008
#define IDC_PARTNv		   7009
#define IDC_LOTBNv         7010
#define IDC_UIICON		   7011
#define IDC_UID06CON1FIRST 7012
#define IDC_UID06CON1SECOND 7013
#define IDC_RECT			7014
//#define IDC_PINMARKERS     4145 (reused due to IDC_USERLEVEL4 insert above)
//#define IDC_PINMARKERSENAB 4146
//#define IDC_PINMARKERSCHAR 4147
//#define IDC_CWR            4148
//#define IDC_IMFEXT         4149
//#define IDC_IMFOORG        4150
//#define IDC_IMFOEXT        4151
//#define IDC_IMFSW          4152
//#define IDC_IMFAUTOFLIP    4153
//#define IDC_IMFOPPOSITE    4154
//#define IDC_IMFCOLS        4155
#define IDC_XDOTS          4156
#define IDC_YDOTS          4157
#define IDC_QZONE          4158
//#define IDC_ERODE          4159
#define IDC_APPICON        4160
#define IDC_HRPICT         4161
#define IDC_HROVERLAY      4162
#define IDC_HROVERLAY1     4163
#define IDC_HROVERLAY2     4164
#define IDC_SUBTYPE        4165
#define IDC_VALUE          4166
#define IDC_REDIT          4167
#define IDC_INCLUDE        4168
#define IDC_GLOBAL         4169
#define IDC_FORMAT         4170
#define IDC_HINTS          4171
#define IDC_CANOVERRIDE    4172
#define IDC_DAYSOFFSET     4173
#define IDC_HOURSOFFSET    4174
#define IDC_MINUTESOFFSET  4175
#define IDC_SECONDSOFFSET  4176
#define IDC_RETRO          4177
#define IDC_OVERRIDE       4178
#define IDC_USENAMES       4179
#define IDC_IS24           4180
#define IDC_ISLZ           4181
#define IDC_SEPERATOR      4182
#define IDC_AMSUFFIX       4183
#define IDC_PMSUFFIX       4184
#define IDC_SHOWSECONDS    4185
#define IDC_FILENAME       4186
#define IDC_DESC           4187
#define IDC_FNLIST         4188
#define IDC_GORDER         4189
#define IDC_GORG           4190
#define IDC_GNSIZE         4191
#define IDC_GNSPACING      4192
#define IDC_GLR            4193
#define IDC_GRL            4194
#define IDC_GTB            4195
#define IDC_GBT            4196
#define IDC_YQZ            4197
#define IDC_XQZ            4198
#define IDC_DMFORMAT       4199
#define IDC_DMECC          4200
#define IDC_DMCOLS         4201
#define IDC_DMROWS         4202
#define IDC_DMSTYLE        4203
#define IDC_DMBORDER       4204
#define IDC_YSIZE          4205
#define IDC_XSIZE          4206
#define IDC_FONT           4207
#define IDC_FONTDESC       4208
#define IDC_BASE           4209
#define IDC_C128CSET       4210
#define IDC_SEF            4211

#define IDC_XLBL       4212
#define IDC_YLBL       4213
#define IDC_XRESLBL    4214
#define IDC_YRESLBL    4215
#define IDC_XDOTSLBL   4216
#define IDC_YDOTSLBL   4217
#define IDC_XSIZELBL   4218
#define IDC_YSIZELBL   4219
#define IDC_XYDOTSLBL  4220
#define IDC_HEIGHT     4221

#define IDC_UNIQUE     4222

#define IDC_IMVERSION  4223
#define IDC_IMAPIVER   4224
#define IDC_IMLICEXP   4299  //sam for lic exp date

#define IDC_JOG        4225
#define IDC_MOVE       4226

#define IDC_YORGM      4227
#define IDC_YINCM      4228
#define IDC_ROT        4229
#define IDC_SLANT      4230
#define IDC_ROTLBL     4231
#define IDC_DEGLBL	   4294 //Degree for rotary -- sam
#define IDC_DEG		   4295 //Degree for rotary -- sam

#define IDC_SLANTLBL   4232

#define IDC_LOCATOR    4233
#define IDC_LOCATEPREP 4234
#define IDC_NACQPOS    4235
#define IDC_ACQPOS0    4236
#define IDC_ACQPOS1    4237
#define IDC_ACQMOVE0   4238
#define IDC_ACQMOVE1   4239
#define IDC_ACQJOG0    4240
#define IDC_ACQJOG1    4241
#define IDC_REFPOS0    4242
#define IDC_REFPOS1    4243
#define IDC_LEARNREFPOS 4244	//Legacy see IDC_LEARNREFPOS0 and IDC_LEARNREFPOS1 - d.dixon 1/15/14
#define IDC_MAXROTATION 4245
#define IDC_MAXLENDELTA 4246
#define IDC_MASKXROT    4247
#define IDC_MASKYROT    4248
#define IDC_ADJUSTA     4249
#define IDC_CALPOS      4250
#define IDC_CALMARK     4251
#define IDC_LOCPOS      4252
#define IDC_LOCATE      4253
#define IDC_DELTAPOS    4254
#define IDC_UPDATE      4255
#define IDC_CALLIST     4256
#define IDC_AUTOCAL     4257
#define IDC_STDLOCPOS   4258
#define IDC_STDLOCMOVE  4259
#define IDC_Z           4260
#define IDC_LOCSIZE     4261
#define IDC_STDLOCPROMPT 4262
#define IDC_FA          4263
#define IDC_ADDSUM      4264
#define IDC_INTERLEAVE  4265
#define IDC_BBARS       4266
#define IDC_CALINST     4267
#define IDC_VEL         4268
#define IDC_KEEPON      4269
#define IDC_BFI         4270
#define IDC_CDEVVIEW    4271
#define IDC_BUFSTAT     4272
#define IDC_ZLBL        4273
#define IDC_JUSTLBL     4274
#define IDC_MSLBL       4275
#define IDC_REPLBL      4276
#define IDC_NREFPOS     4277

#define IDC_FXCOR       4278
#define IDC_FYCOR       4279
#define IDC_FSCOR       4280
#define IDC_RXCOR       4281
#define IDC_RYCOR       4282
#define IDC_RSCOR       4283
//#define IDC_BITMAPFILE  4284          //-------Linghai-11-12-08-----
#define IDC_FLBL        4284              //------------Linghai-5-28-09---
#define IDC_ELBL        4285              //------------Linghai-5-28-09---
#define IDC_QUITPG       4286
#define IDC_EXITPG       4287
#define IDC_EDITL          4288
#define IDC_EDITW          4289
#define IDC_EDITH          4290
#define IDC_BROWSELINK     4291
#define IDC_EDIT1LINK      4292
#define IDC_LINKDISPLAY	   4293

#define IDC_QRROT 4300
#define IDC_QRENC 4301
#define IDC_QRCOR 4302
#define IDC_QRVER 4303
#define IDC_QRDEG 4304		//--sam	10/10/16

#define IDC_COPYREFPOS2CAM 4400
// Additional AE changes
#define IDC_cbLOCATOR	4990		//----d.dixon----11/17/13--- safely placed here out of order because
#define IDC_cbLOCATOR1  4991
#define IDC_cbLOCATOR2  4992
#define IDC_LOCATEPREP1 4993
#define IDC_LOCATEPREP2 4994
#define IDC_cbPROCTYPE1 4995
#define IDC_cbPROCTYPE2 4996
#define IDC_LEARNREFPOS0 4997
#define IDC_LEARNREFPOS1 4998		//---- the previous ID's are often used with indexing method(ie. IDC_ID+1, etc)

// Menu IDs
#define IDM_ABOUT             5000
#define IDM_EXIT              5001
#define IDM_SHELL             5002
#define IDM_QUIT              5003
#define IDM_OPEN              5004
#define IDM_NEWIMP            5005
#define IDM_NEWIMF            5006
#define IDM_RUN               5007
#define IDM_DELETE            5008
#define IDM_SAVE              5009
#define IDM_SAVEAS            5010
#define IDM_TEST              5011
#define IDM_NEWKVAR           5012
#define IDM_NEWSVAR           5013
#define IDM_NEWTEXTMARK       5014
#define IDM_NEWDEVBCMARK      5015
#define IDM_NEWBITMAPMARK     5016
#define IDM_CFGSAVE           5017
#define IDM_CFGSHELL          5018
// #define IDM_CFGFIXORG      5019
#define IDM_USERS             5020
#define IDM_PREVIEW           5021
//                            5022
//                            5023
//                            5024
//                            5025
//                            5026
//                            5027
#define IDM_NEWC2OF5MARK      5028
#define IDM_NEWC39MARK        5029
#define IDM_NEWC128MARK       5030
#define IDM_NEWC93MARK        5032
#define IDM_SHUTDOWN          5033
#define IDM_CLEAN             5034
#define IDM_POWERDOWN         5035
#define IDM_PRINTON		      5036
#define IDM_DDERUNSTART       5044 // posted from DDERunExecute
#define IDM_NEWBASEDVAR       5045
#define IDM_NEWCUPCEANMARK    5046
#define IDM_NEWUDHA           5047
#define IDM_NEWMULTILINEMARK  5048
#define IDM_NEWDVAR           5049
#define IDM_NEWTVAR           5050
#define IDM_SUPPRESSLINK      5051
#define IDM_SUPPRESSLINKAUTOSTART 5052
#define IDM_SUPPRESSCONVEYOR  5053
#define IDM_SUPPRESSINCREMENT 5054
#define IDM_STOPONFAIL        5055
#define IDM_NEWCDMMARK        5056
#define IDM_NEWWINFONTMARK    5057
#define IDM_NEWPATCHMARK      5058
#define IDM_EDITAE            5059
#define IDM_DELETEAE          5060
#define IDM_CAL               5061
#define IDM_SUPPRESSALIGN     5062
#define IDM_NEWCURESTEP       5063
#define IDM_NEWCDEVMARK       5064
#define IDM_EDITCDEVBLOCK     5065
#define IDM_DELCDEVBLOCK      5066
#define IDM_EDITIMC           5067
#define IDM_DELETEIMC         5068
#define IDM_NEWFORMAT12      5069                   //--------------Linghai-11-21-08---
#define IDM_NEWFORMAT6       5070                   //--------------Linghai-11-21-08---
#define IDM_NEWFORMAT5       5071                   //--------------Linghai-11-21-08--
#define IDM_NEWUIDMARK       5072                   //--------------Linghai-4-9-09---
#define IDM_NEWNOTES	     5073
#define IDM_OPENNOTES	     5074
#define IDM_APROCEDURE       5075
#define IDM_OPROCEDURE       5076
#define IDM_NEWWMTRXMARK     5077
#define IDM_NEWCQRMARK       5078

#define IDM_CUT              5100
#define IDM_COPY             5101
#define IDM_PASTE            5102
#define IDM_PASTEAFTER       5103

// Special IDM (internal:no menu)
#define IDM_LINKSTART  5500
#define IDM_LINKSKIP   5501


// Accelerator IDs
#define IDA_START           6000
#define IDA_MVE             6009  //----sam----testing for rotary
#define IDA_INIT            6001
#define IDA_CANCEL          6002
#define IDA_QTY             6003
#define IDA_ALL             6004
#define IDA_EXTRA1          6005
#define IDA_EXTRA2          6006
#define IDA_EXTRA3          6007
#define IDA_CLAMPREL        6008
	
#ifndef RC_INVOKED // Hide the rest from rc

#include <jvmath32\jvmath32.h>

#define TSIZE 1024 // sizeof(tbuf)

#define UHISTVALSIZE (IMPLKVVAL+1+IMPLVNAME+1+2)

/******************************************************************************
    UTILITY MACROS
******************************************************************************/
#define DELZ(p) {delete p;p=NULL;} // delete and zero

#define IERROR \
    {IDError=IDE_INTERNAL;iErrorLine=__LINE__;iErrorFile=__FILE__;}

// Delete GDI objects and 0 the handle
#define DELOBJ(o) {if(o) {DeleteObject(o);o=0;}}

// lampcodes for SetLamps()
#define NOLAMP 0
#define GREENLAMP 1
#define REDLAMP 2

/******************************************************************************
    PUBLIC TYPES
******************************************************************************/


// Application mode
typedef enum {mLOGIN,mFILES,mEDIT,mRUN,mIMFEDIT,mFILES1} IMSMODE;

/******************************************************************************
    PUBLIC VARIABLES
******************************************************************************/
extern HINSTANCE IMSInst;  // IMS application instance handle
extern HWND IMSWnd;     // IMS application window
extern HWND IMWnd;      // IM child window
extern char IMSName[];  // IMS application name
extern IMSMODE IMSMode; // Current application mode
extern HWND IMSADWnd;   // Active Dialog Window for IsDialogMsg...
extern char IMSDir[];   // IMS directory
extern char IMSIni[];   // IMS application .INI file name.
extern char CalIni[];   // IMS calibration .INI file name.
extern char CalHistIni[];   // IMS calibration history .INI file name.
extern char IMIni[];    // IM5000 .INI file name for cal updates.
extern char IMSUsr[];   // IMS application .USR file name.
extern char IMSProgs[]; // IMS programs directory
extern char IMSFixtures[]; // IMS Fixtures directory
extern char BMPDir[];   // BitmapDir directory
extern char IMSVisObjects[]; // IMS Vision Objects directory
extern BOOL IMSIsShell; // true if this instance is the shell
extern BOOL IMSIsActive; // true if IMS is active app
extern char var1[TSIZE];
extern char var2[TSIZE];
extern char var1val[TSIZE];
extern char var2val[TSIZE];
extern int var1len;
extern int var2len;
extern int var1vallen;
extern int var2vallen;
extern char tbuf[TSIZE];  // General purpose text buffer
extern char tbuf1[TSIZE];
extern char tbuf2[TSIZE];
extern char rbuf[TSIZE];  //sam General purpose text buffer
extern char rbuf1[TSIZE]; //sam
extern int tlen;         // Length of tbuf
extern int tlen1;
extern int tlen2;
extern TEXTMETRIC sftm; // System font text metrics
extern TEXTMETRIC sfftm; // System fixed font text metrics
extern WORD IDError;     // Error ID
extern int iErrorLine;     // Line number ref for internal errors
extern char *iErrorFile;   // Filename ref for internal errors
extern WORD IMState;       // The current IM State IMN_ 
extern WORD UniqueOverrideLevel; // The min user level to override uniqe warns
// Other ini vars
extern P3 RehomePoint;     // Head position prior to re-home
extern P3 StartPoint;      // Head position for start and end of marking
extern P1 rotrehomepoint;            // Head position prior to re-home   ---sam---
extern P1 rotstartpoint;             // Head position for start and end of marking  ----sam----
extern BOOL ZStartOnly;    // Use StartPoint.z only flag
extern BOOL ReturnToStart; // Return to starting point after marking
extern BOOL UpSpray;       // Draw IMF in upspray mode flag
extern BOOL VisEnable;     // True if vision can be enabled by DDE
extern BOOL VisInterleave; // True if acquire interleaved with marking
extern BOOL ReplaceZeros;  // True if '0' to be replaced with 'O'
extern BOOL WarnOnBlankVars; // Enables blank variables on start button warn
extern BOOL Metric;        // True if metric (cm) instead of inch mode
extern BOOL MetricMM;      // Flag for mm instead of cm for Metric mode
extern BOOL NoFontMarks;   // Disable all font based marks from edit new menu
extern BOOL NoBitmapMarks; // Disable all bitmap based marks from edit new menu
extern IMINFO IMInfo;      // Info about the IM

#define YEXISTS (IMInfo.extent.y)
#define ZEXISTS (IMInfo.extent.z)
#define WEXISTS (IMInfo.extent.d)  //sam

// HideOpt
extern BOOL HideOptRotation;
extern BOOL HideOptInspect;
extern BOOL HideOptRead;
extern BOOL HideOptConveyor;
extern BOOL HideOptInit;
extern BOOL HideOptLink;
extern BOOL HideOptRotaryIMF;
extern BOOL HideOptRotaryDEG;   // ---------for part rotary degree-----sam 09-20-2016
extern BOOL HideOptMarkRotation;
extern BOOL HideOptMarkSlant;

// Vision DDE vars
extern BOOL VisOn; // enables mark+acquire cycle
extern BOOL updateVar;
extern P3 VisPos;  // servo position for camera in part coordinate system

// LQTY system variable for AUX system
extern UINT AuxLQty; // %@LQTY% loaded by PromptForLQty=1 in .AUX file.

// Conveyor
extern int	ConvType;
extern int	ConvDelay;
extern BOOL ConvOpen;
extern BOOL ConvSimplimatic; // True if new style PIP PR PC

// New style ConvSimplimatic bits
extern HBIT bitConvRun;   // Output enables conveyor operation
extern HBIT bitConvReset; // Output resets conveyor
extern HBIT bitConvReady; // Input indicates conveyor ready
extern HBIT bitConvError; // Input indicates conveyor error
extern HBIT bitConvPIP1;  // Input indicates Part In Position 1
extern HBIT bitConvPR1;   // Output Part Request 1
extern HBIT bitConvPC1;   // Output Part Complete 1
extern HBIT bitConvPIP2;  // Input indicates Part In Position 2
extern HBIT bitConvPR2;   // Output Part Request 2
extern HBIT bitConvPC2;   // Output Part Complete 2
extern HBIT bitConvPIP3;  // Input indicates Part In Position 3
extern HBIT bitConvPR3;   // Output Part Request 3
extern HBIT bitConvPC3;   // Output Part Complete 3
extern char sConvRun[];
extern char sConvReset[];
extern char sConvReady[];
extern char sConvError[];
extern char sConvPIP1[];
extern char sConvPR1[];
extern char sConvPC1[];
extern char sConvPIP2[];
extern char sConvPR2[];
extern char sConvPC2[];
extern char sConvPIP3[];
extern char sConvPR3[];
extern char sConvPC3[];

// Old style !ConvSimplimatic bits
extern HBIT bitConvPIP; // Input signals part in position
extern HBIT bitConvPC;  // Output signals part complete/request
extern HBIT bitConvPS;  // Output signals PASSED status on 0->1 PC
extern HBIT bitThruConvPIP; // Input signals part in position at thru section
extern HBIT bitThruConvPC;  // Output signals part complete/request at thru section
extern HBIT bitThruConvPC1;
extern HBIT bitThruConvPC2;
extern HBIT bitThruConvPC3;
extern HBIT bitConvOutGate; // Mask downstream avail when off to fix PCT bug
extern char sConvPIP[]; // bit names
extern char sConvPC[];
extern char sConvPS[];
extern char sThruConvPIP[];
extern char sThruConvPIP1[];
extern char sThruConvPIP2[];
extern char sThruConvPIP3[];
extern char sThruConvPC[];
extern char sThruConvPC1[];
extern char sThruConvPC2[];
extern char sThruConvPC3[];
extern char sConvOutGate[];

extern BOOL SuppressConveyor; // Controlled by menu entry
extern BOOL LinkFail;      // True if linking IMP is reporting FAIL
extern BOOL CycleComplete;  // TRUE if the parts are done
extern BOOL CycleFailed;    // TRUE if not all parts passed
extern BOOL CycleStarted;    // TRUE if any parts have been started

// RedGreen Lamps
extern BOOL RedGreenOpen;
extern HBIT bitRedLamp;
extern HBIT bitGreenLamp;
extern char sRedLamp[];
extern char sGreenLamp[];
extern BOOL RedLampTrapError;

// CIMple support
extern BOOL CIMpleIsOpen;
extern BOOL CIMpleTrapError;

extern char sDDERUN[];
//extern JETDBO32 * dbObj;
extern BOOL DebugMode;

/******************************************************************************
    PUBLIC FUNCTION PROTOTYPES
******************************************************************************/
/******************************************************************************
    FAR PUBLIC FUNCTION PROTOTYPES
******************************************************************************/
void SetMode(IMSMODE newmode);
void SetVar(char *var1,char *val1,char *var2,char *val2);
BOOL chddir(LPSTR ddir);
void getddir(LPSTR ddir);
DWORD _lread32(HANDLE hFile,LPVOID lpBuf,DWORD dwBytes);
DWORD _lwrite32(HANDLE hFile,LPCSTR lpBuf,DWORD dwBytes);
DWORD _llseek32(HANDLE hFile,LONG lOffset,DWORD iOrigin);
char * FindFirstFile(char *pattern);
char * FindNextFile(void);

BOOL DoPrompt(HWND parent,LPSTR name,WORD length,LPSTR def);
BOOL DoPromptO(HWND parent,LPSTR name,WORD length,LPSTR def,LPSTR def1);
BOOL DoPromptH(HWND parent,LPSTR name,WORD length,LPSTR def,LPSTR def1); //for part dia and height
BOOL DoPromptP(HWND parent,LPSTR name,WORD length,LPSTR def,LPSTR def1,LPSTR def2);
BOOL DoPromptPD(HWND parent,LPSTR name,WORD length,LPSTR def,LPSTR def1);
BOOL DoFixtureFile(HWND parent, LPSTR name, WORD length, LPSTR def);
void ShowError(WORD ide);
void ShowWarning(WORD idw,LPCSTR s);
int IDSMessageBox(WORD ids,WORD flags);
int GetDate(LPSTR buf,LPSTR fmt,long ofs,LPSTR base);
int GetTime(LPSTR buf,BOOL useNames,BOOL is24,
	BOOL isLZ,char seperator,LPSTR amSuffix,LPSTR pmSuffix,
	BOOL showSeconds,long offset,LPSTR base);
void GetGMT(LPSTR buf);

WORD VectToString(LPSTR s,LPDOUBLE v,int n,LPSTR sep);
void MScaleVect(LPDOUBLE v,int n,BOOL inv);
void XYMScaleVect(LPDOUBLE ans,LPDOUBLE v);
void XMScaleVect(LPDOUBLE ans,LPDOUBLE v);
void YMScaleVect(LPDOUBLE ans,LPDOUBLE v);

void IMSGetDefDirs(void);

// Red Green lamp system
void RedGreenInit(void);
void SetLamps(int lampcode);

// CIMple system
void CIMpleInit(void);
void CIMpleQuit(void);
void CIMpleSendError(WORD ide,LPSTR module,LPSTR text);
void CIMpleSendMark(void);

// Unique history functions
BOOL UHISTInit(void);
void UHISTFree(void);
BOOL UHISTAdd(LPSTR s);
BOOL UHISTFind(LPSTR s);
void UHISTClear(void);

void SetFloatField(HWND hdlg,int idc,double f);
BOOL GetFloatField(HWND hdlg,int idc,LPDOUBLE f);

#endif // RC_INVOKED

#endif // _IMS5000_H
