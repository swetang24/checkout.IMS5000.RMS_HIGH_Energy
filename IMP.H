#ifndef _IMP_H
#define _IMP_H

/***************************************************************************
	Name : IMP.H

	Desc : Definition of IMS Inkjet Marking Program structure and
	       functions to support it.
***************************************************************************/
#include <dm00132\dm00132.h>
#include <im5000\im5000.h>
#include "imsconst.h"
#include "ae.h"
#include "imcorr.h"

/***************************************************************************
	PUBLIC TYPES
***************************************************************************/

#pragma warning(disable : 4200) // disable warnings about empty array in struct

class IMPHEADER
	{
public:
	char desc[IMPLDESC+1];       // Description text
	char author[IMPLAUTHOR+1];   // Authors name
//	char fixture[IMPLFIXTURE+1]; // Fixture base filename
	char fixture[128+1]; // RESERVED for when IMPLFIXTURE=128
	P3 partSize;                 // Part size
	P1 partDiameter;                 // Part diameter-------sam
	

	IJDEVICENAME inkjet;         // Name of IJ device
	P2 origin;                   // FaceCS Origin
	double rotation;              // FaceCS Theta
	double degree;				//Rotation degree	--sam	10/7/16
	char initsect[IMPLFNBASE+1]; // init ini section or ""
	char link[IMPLFNBASE+1];     // link imp base filename or ""
	BOOL inconlink;              // increment enable when linking
	char inspector[IMPLFNBASE+1];// Inspector or "" for no inspection
	P3 inspectpos;               // Inspection position relative part origin.
	P4 rotinspectpos;               // Inspection position relative part origin.------sam---
	char inspectprep[IMPLVISPREP+1]; // inspetion prep expression
	BOOL conveyorstart;          // SMEMA conveyor start mode enable
	BOOL linkautostart;          // Auto start on linked run 
	BOOL skiponlinkfail;         // Don't skip marking on LinkFail&RunLinked
	BOOL chainlinkfail;          // OR CycleFail into LinkFail
	double zflight;               // ZFlight override if >0 //RESERVED
	char reader[IMPLFNBASE+1];    // Reader or "" for no read
	P3 readpos;                   // Read position relative part origin.
	P4 rotreadpos;                   // Read position relative part origin.-----sam-----
	char readprep[IMPLVISPREP+1]; // optional read prep expression or ""
	char readvar[IMPLVNAME+1];    // Name of instance var to read info into
	IMPHEADER();     // Fill in members with defaults
	BOOL write(HANDLE hFile);
	BOOL read(HANDLE fh, WORD ver);
private:
	BOOL readnative(HANDLE fh, WORD ver);
	BOOL translate(HANDLE fh, WORD ver);
	BOOL translateVers343Thru368(HANDLE fh, WORD ver);
	BOOL translateVer341(HANDLE fh);
	BOOL translateVer338(HANDLE fh);
	BOOL translateOriginalVersion(HANDLE fh);
	// Version 3.30 and original version are the same
	};

class IMPUDHA // User Defined Header Attribute
	{
public:
	friend class IMPUDHAS;
	char name[IMPLUDHANAME+1];   // Name of the variable
	BYTE redit;                  // True if runtime edit supported
	char val[IMPLUDHAVAL+1];     // Value of the variable
	
	IMPUDHA();     // Fill in members with defaults
	~IMPUDHA(); 

	BOOL read(HANDLE fh, WORD ver);
	BOOL write(HANDLE fh);
private:
	BOOL readnative(HANDLE fh, WORD ver);
	BOOL translate(HANDLE fh, WORD ver);
	};

class IMPUDHAS // User Defined Header Attributes
	{
public:
	
	WORD count;      // number of IMPUDHA s  0..IMPMAXUDHAS
	WORD capac;      // Capacity - number of elements allowed
	IMPUDHA * udha; // count IMPUDHA structs
	IMPUDHAS(const IMPUDHAS & src);
	IMPUDHAS& operator= (const IMPUDHAS & src);
	IMPUDHAS();
	IMPUDHAS(int initCapac);
	~IMPUDHAS();
	void free();
	BOOL realloc(int ncount);
	BOOL write(HANDLE hFile);
	BOOL read(HANDLE fh, WORD ver, WORD count);
private:
	BOOL readnative(HANDLE fh, WORD ver, WORD count);
	BOOL translate(HANDLE fh, WORD ver, WORD count);
	};


struct IMPDATEVAL
	{
	char override[IMPLDATEOVERRIDE+1]; // Date override if ! ""
	};

struct IMPTIMEVAL
	{
	char override[IMPLTIMEOVERRIDE+1]; // Time override if ! ""
	};

union IMPVAL
	{
	char k[IMPLKVVAL+1]; // Value if its a keyed var
	struct
		{
		DWORD s;    // Value if its a serial var
		WORD c;     // Repeat count if its a serial var
		};
	IMPDATEVAL d;        // Value if its a date var
	IMPTIMEVAL t;        // Value if its a time var
	char b[IMPLBASEDVAL+1]; // Value if its a based var
	};

class IMPVALS
	{
public:
	WORD count;      // number of vals = vars.count
	WORD capac;      // current capacity
	IMPVAL * val; // count IMPVAL structs
	IMPVALS(const IMPVALS & src);
	IMPVALS& operator= (const IMPVALS & src);
	IMPVALS();
	IMPVALS(int initCapac);
	~IMPVALS();
	void free();
	};

// 3.43-native
typedef enum {vtKEYED,vtSERIAL,vtDATE,vtTIME,vtBASED} IMPVARTYPE;
// 3.41
typedef enum {vtV341KEYED,vtV341SERIAL,vtV341DATE,vtV341TIME,vtV341BASED} IMPV341VARTYPE;
// 3.30-3.38
typedef enum {vtV338KEYED,vtV338SERIAL,vtV338DATE,vtV338TIME,vtV338B40} IMPV338VARTYPE;
// Original version
typedef enum {vt4001KEYED,vt4001SERIAL,vt4001B40} IMP4001VARTYPE;

struct IMPKEYEDVAR			  // 20 bytes
	{
	BOOL digits;              // Variable can contain Digits
	BOOL lowers;              // Variable can contain Lowercase Letters
	BOOL uppers;              // Variable can contain Uppercase Letters
	BOOL space;               // Variable can contain Space
	BOOL others;              // Variable can contain other characters
	};

struct IMPSERIALVAR	 // 24 bytes
	{
	WORD  repno;     // Current repeat number
	DWORD step;      // Step size
	BOOL  dirpos;    // Count direction positive or FALSE for negative
	WORD  repeat;    // Repeat limit 0..9999
	DWORD minval;    // Minimum value 0 to IMPMAXSVVAL
	DWORD maxval;    // Maximum value 0 to IMPMAXSVVAL
	BOOL  loop;      // Program either loops or stops
	};

struct IMPBASEDVAR  // 25 bytes
	{
	char base[IMPLBASEDNAME+1]; // name of base from ini file
	BOOL loop;      // Sequence either loops or stops
	};

struct IMPDATEVAR   // 50 bytes
	{
	long offset;                   // date offset in seconds
	char format[IMPLDATEFORMAT+1]; // Date format string
	BOOL canOverride;  // True if user can override value at runtime
	char base[IMPLBASEDNAME+1]; // name of base from ini file (10 if null)
	};

struct IMPTIMEVAR   // 58 bytes
	{
	long offset;    // clock offset in seconds (-86400<offset<86400)
	BOOL useNames;  // Use time names from ini
	BOOL is24;      // True if 24 hour clock (!useNames)
	BOOL isLZ;      // True if leading zero on hour (!useNames)
	char seperator; // seperator character (!useNames)
	char amSuffix[IMPLTIMESUFFIX+1]; // 12hr AM suffix (!useNames)
	char pmSuffix[IMPLTIMESUFFIX+1]; // 12hr PM|24hr suffix (!useNames)
	BOOL showSeconds; // True if seconds should be displayed (!useNames)
	BOOL canOverride; // True if user can override value at runtime
	char base[IMPLBASEDNAME+1]; // name of base from ini file (10 if null)
	};

// For versions 3.43 to 3.68 - vars had 15 bit type and 1 bit unique
struct IMPV343VARTYPE
	{
	short type: 15;
	short unique: 1;
	};

class IMPVAR
	{
public:
	friend class IMPVARS;
	char name[IMPLVNAME+1];   // Variable name
	int width;               // Max width of variable
	char lpad;                // if(lpad) left pad to width with lpad
	IMPVARTYPE type;     // variable type - used to be 15 bits
	BOOL unique;		 // variable type (no duplicates in cycle) - was 1 bit
	IMPVAL preval;            // Preview value
	union
		{
		IMPKEYEDVAR  k;           // type==vtKEYED
		IMPSERIALVAR s;           // type==vtSERIAL
		IMPDATEVAR   d;           // type==vtDATE
		IMPTIMEVAR   t;           // type==vtTIME
		IMPBASEDVAR  b;           // type==vtBASED
		};
	IMPVAR();     // Fill in members with defaults
	~IMPVAR();

	BOOL read(HANDLE fh,WORD ver);
	BOOL write(HANDLE fh);
	void initByType();
private:
	BOOL readnative(HANDLE fh, WORD ver);
	BOOL translate(HANDLE fh, WORD ver);
	BOOL translateVers343Thru368(HANDLE fh, WORD ver);
	BOOL translateVer341(HANDLE fh);
	BOOL translateVers330Thru338(HANDLE fh);
	BOOL translateOriginalVersion(HANDLE fh);
	};

class IMPVARS
	{
public:
	WORD count;      // number of vars
	WORD capac;      // current capacity
	IMPVAR * var; // count IMPVAR structs
	IMPVARS(const IMPVARS & src);
	IMPVARS& operator= (const IMPVARS & src);
	IMPVARS();
	IMPVARS(int initCapac);
	~IMPVARS();
	void free();
	BOOL realloc(int ncount);
	BOOL write(HANDLE hFile);
	BOOL read(HANDLE fh, WORD ver, WORD count);
private:
	BOOL readnative(HANDLE fh, WORD ver, WORD count);
	BOOL translate(HANDLE fh, WORD ver, WORD count);
	};

typedef enum 
	{mtTEXT,mtDEVBC,mtBITMAP,mtC128,mtCUPCEAN,mtC93
	,mtMULTILINE,mtCDM,mtWINFONT,mtPATCH,mtC39,mtC2OF5,mtCURE
	,mtCDEV,mtDEVSYM,mtUID
// New Marks since 4.0
	,mtWMTRX
	,mtCQR
	} IMPMARKTYPE;

// Marks for version 4.0
typedef enum {mtV400TEXT,mtV400DEVBC,mtV400BITMAP,mtV400C128,mtV400CUPCEAN,mtV400C93,
	mtV400MULTILINE,mtV400CDM,mtV400WINFONT,mtV400PATCH,mtV400C39,mtV400C2OF5,mtV400CURE,
	mtV400CDEV,mtV400DEVSYM,mtV400UID} IMPV400MARKTYPE;                                  //------------Linghai-6-19-09---
/*
// Marks for version 3.43 to native
typedef enum {mtTEXT,mtDEVBC,mtBITMAP,mtC128,mtCUPCEAN,mtC93,
	mtMULTILINE,mtCDM,mtWINFONT,mtPATCH,mtC39,mtC2OF5,mtCURE,
	mtCDEV,mtDEVSYM} IMPMARKTYPE;                            //------------Linghai-11-21-08---
*/	
// Version 3.41 mark types
typedef enum {mtV341TEXT,mtV341DEVBC,mtV341BITMAP,mtV341C128,mtV341CUPCEAN,
	mtV341C93,mtV341UCC128,mtV341MULTILINE,mtV341CDM,mtV341WINFONT,
	mtV341PATCH} IMPV341MARKTYPE;
// Version 3.38 mark types
typedef enum {mtV338TEXT,mtV338DEVBC,mtV338BITMAP,mtV338C128,mtV338CUPCEAN,
	mtV338C93,mtV338UCC128,mtV338MULTILINE,mtV338CDM,mtV338WINFONT,
	mtV338PATCH} IMPV338MARKTYPE;
// Version 3.30 and earlier
typedef enum {mtV330TEXT,mtV330DEVBC,mtV330BITMAP,mtV330C128,mtV330CUPCEAN,
	mtV330C93,mtV330UCC128,mtV330MULTILINE} IMPV330MARKTYPE;

typedef enum {mjxLEFT,mjxCENTER,mjxRIGHT} IMPMARKJUSTX;
typedef enum {mjyTOP,mjyMIDDLE,mjyBOTTOM} IMPMARKJUSTY;

struct IMPTEXTMARK
	{
	IJFMT ijfmt;           // Inkjet format (font,ms,xflip,yflip)
	double cw;              // Character width
	double ch;              // Character cell height
	char text[IMPLTEXT+1]; // Mark text
	};

struct IMPDEVBCMARK
	{
	IJFMT ijfmt;             // Inkjet format (font,ms,xflip,yflip)
	double cw;                // Character width
	double ch;              // Character cell height
	char text[IMPLDEVBC+1]; // DEVBC text
	};

struct IMPDEVSYMMARK
	{
	IJFMT ijfmt;             // Inkjet format (font,ms,xflip,yflip)
	double width;            // X width of the mark in inches (not including ms)
	double height;           // Y height of the mark in inches
	WORD xdots;              // X dots of the mark
	char text[IMPLDEVSYM+1]; // Mark text
	};

// IMPC128MARK.cset values
typedef enum {C128OPT=0,C128SETA,C128SETB,C128SETC,C128UCC} C128CSET;

// IMPC128MARK.overlay values
typedef enum {HRNONE=0,HRTOP,HRBOTTOM} HRMODE; // Code HR 

struct IMPC128MARK
	{
	double xres;           // X resolution in drops/inch (not including ms)
	double yres;           // Y resolution in drops/inch
	int dx;               // Dots per module
	int dy;               // Dots per bar y
	int qz;               // Quiet Zone width in modules
	WORD ms;              // Multistroke factor
	BOOL neg;             // Negative code128 flag
	HRMODE overlayhr;     // The human readable overlay value
	char text[IMPLC128+1]; // C128 text
	char hrpict[IMPLC128HRPICTURE+1]; // Human readable picture "(X) XC" 
	BYTE cset; /*: 3; */        // char set - used to be 3 bits
	BYTE reserved; /* : 5; */   // reserved - used to be 5 bits
	};

struct IMPCUPCEANMARK
	{
	double xres;           // X resolution in drops/inch (not including ms)
	double yres;           // Y resolution in drops/inch
	int dx;               // Dots per module
	int dy;               // Dots per bar y
	int qz;               // Quiet Zone width in modules
	WORD ms;              // Multistroke factor
	BOOL neg;             // Negative flag
	int  subtype;         // UPCA|UPCE|EAN8
	BOOL overlayhr;       // Overlay human readable
	char hrpict[IMPLCUPCEANHRPICTURE+1]; // Human readable picture "(X)-XC" 
	char text[IMPLCUPCEAN+1]; // UPCEAN text
	};

struct IMPC93MARK
	{
	double xres;           // X resolution in drops/inch (not including ms)
	double yres;           // Y resolution in drops/inch
	int dx;               // Dots per module
	int dy;               // Dots per bar y
	int qz;               // Quiet Zone width in modules
	WORD ms;              // Multistroke factor
	BOOL neg;             // Negative code93 flag
	char text[IMPLC93+1]; // C93 text
	};

struct IMPC39MARK
	{
	double xres;           // X resolution in drops/inch (not including ms)
	double yres;           // Y resolution in drops/inch
	int dx;               // Dots per module
	int dy;               // Dots per bar y
	int qz;               // Quiet Zone width in modules
	WORD ms;              // Multistroke factor
	BOOL neg;             // Negative code128 flag
	HRMODE overlayhr;     // The human readable overlay type
	BOOL fa;              // Full ASCII mode
	BOOL addsum;          // Compute and add checksum
	char text[IMPLC39+1]; // C39 text
	char hrpict[IMPLC39HRPICTURE+1]; // Human readable picture "(X) XC" 
	};

struct IMPC2OF5MARK
	{
	double xres;             // X resolution in drops/inch (not including ms)
	double yres;             // Y resolution in drops/inch
	int dx;                 // Dots per module
	int dy;                 // Dots per bar y
	int qz;                 // Quiet Zone width in modules
	WORD ms;                // Multistroke factor
	BOOL neg;               // Negative code128 flag
	HRMODE overlayhr;       // The human readable overlay type
	BOOL interleave;        // Interleave 2of5 vs descreet
	BOOL bbars;             // Add baerer bars
	BOOL addsum;            // Add checksum
	char text[IMPLC2OF5+1]; // text
	char hrpict[IMPLC2OF5HRPICTURE+1]; // Human readable picture "(X) XC" 
	};

struct IMPUIDMARK
{
	double xres;           // X resolution in drops/inch (not including ms)
	double yres;           // Y resolution in drops/inch
	WORD ms;              // Multistroke factor
	DMINFO dminfo;        // DM001 Info
	char text[IMPLCDM+1]; // uncompiled text
	};

struct IMPCDMMARK
	{
	double xres;           // X resolution in drops/inch (not including ms)
	double yres;           // Y resolution in drops/inch
	WORD ms;              // Multistroke factor
	DMINFO dminfo;        // DM001 Info
	char text[IMPLCDM+1]; // uncompiled text
	};

struct IMPWINFONTMARK
	{
	double xres;      // X resolution in drops/inch (not including ms)
	double yres;      // Y resolution in drops/inch
	WORD ms;         // Multistroke factor
	BOOL neg;        // Negative image
	LOGFONT logfont; // The Windows logical font
	char text[IMPLWINFONT+1]; // uncompiled text
	BYTE sef;        // space expansion factor
	int yorgm; // yorigin modifier (adds to yorigin)
	int yincm; // leading modifier (0==none) (<0 adds to leading) (>0 replaces leading)
	};

struct IMPWMTRXMARK
	{
	double xres;      // X resolution in drops/inch (not including ms)
	double yres;      // Y resolution in drops/inch
	WORD ms;         // Multistroke factor
	BOOL neg;        // Negative image
	int srcxs,srcys; // source data size in chars
	int subxo,subyo; // sub origin in chars
	int subxs,subys; // sub size in chars
	int celxs,celys; // cell size in pixels
	int dotxs,dotys; // dot size in pixels
	int matxo,matyo; // matrix  origin
	int chxo,chyo;   // col header origin
	int rhxo,rhyo;   // row header origin
	BOOL hex;
	char text[IMPLWMTRX+1]; // uncompiled text
	BYTE flags;
	char chtext[IMPLWMTRXCHTEXT+1]; // col heading text (1 char per column)
	char rhtext[IMPLWMTRXRHTEXT+1]; // row heading text (1 char per row)
	LOGFONT logfont; // The Windows logical font
	};

struct IMPCQRMARK
	{
	double xres;           // X resolution in drops/inch (not including ms)
	double yres;           // Y resolution in drops/inch
	WORD ms;               // Multistroke factor
	BOOL neg;              // Negative image
	int rot; // rotation 0..3 * 90
	double deg;			//degree for rotation	--sam 10/10/16
	int xflip,yflip; // flip bits
	int xmul,ymul; // pixel multiplier
	int xqz,yqz;   // extra quiet zone borders

	IMPCQRENC enc;
	IMPCQRCOR cor;
	int ver;

	char text[IMPLCQR+1]; // uncompiled text
	};

struct IMPPATCHMARK
	{
	double xres;      // X resolution in drops/inch (not including ms)
	double yres;      // Y resolution in drops/inch
	WORD ms;         // Multistroke factor
	BOOL neg;        // Negative image
	POINT size;      // Patch size in pixels x and y
	WORD  res1;      // Reserved for fill style enum
	WORD  res2;      // Reserved for pen style enum
	WORD  res3;      // Reserved for type Rect Ellipse RoundRect
	POINT res4;      // Reserved for round rect corner ellipse
	};

struct IMPBITMAPMARK
	{
	double xres;            // X resolution in drops/inch (not including ms)
	double yres;            // Y resolution in drops/inch
	WORD ms;               // Multistroke 1..IMPMAXMS (not supported!)
	BOOL neg;              // Negative bitmap flag
	char fn[IMPLBMFN+1];   // Bitmap filename
	};

struct IMPMULTILINEMARK
	{
	IJFMT ijfmt;             // Inkjet format (font,ms,xflip,yflip)
	double cw;                // Character cell width
	double ch;                // Character cell height
	char text[IMPLMLTEXT+1]; // Mark text (with \n seperating lines)
	};

// Not really a mark but a uv cure step
// Moves from origin w units along xmark at v with cure on
struct IMPCURESTEP
	{
	double xsize;   // Width of cure line on markx axis (positive)
	double vel;     // Cure velocity in inches per second or 0 for default
	BOOL keepon;   // True if cure lamp should remain on after this step
	};

class IMPMARK;

// Compund Device Mark
struct IMPCDEVMARK
	{
	IJFMT bijfmt; // Base Inkjet Format (font,ms,xflip,yflip)
	int bfi;      // Bitmap format index (of selected IJBMPFMT)
	double xres;   // X resolution in drops/inch (not including ms)
	double yres;   // Y resolution in drops/inch
	char text[IMPLCDEVTEXT+1]; // Mark Text with:
	                           //   \n line seperators (\r ignored)
	                           //   \033Fn inline font changes
	                           //     n=logical(imp) font index + 1
	                           //   \033Bn inline bitmaps
	int nbsm; // number of bmp sub marks
	IMPMARK *bsm[IMPMAXCDEVBSM]; // pointers to bmp sub marks
	};

struct IMPV343C128CSET{
	BYTE cset: 3;
	BYTE reserved: 5;
};

struct IMPV341C128CSET{
	BYTE cset: 2;
	BYTE reserved: 6;
};

class IMPMARK
	{
public:
	friend class IMPMARKS;
	P2 origin;              // Mark origin in program coordinates
	P3 degree;              // rotary Mark origin in program coordinates
	double rotation;         // Mark rotation in degrees from FaceCS
    P3 deg;              // Mark rotation in degrees fr rotary   sam
	double slant;            // Mark y axis slant in degrees from rotated FaceCS
	double zflight;          // ZFlight override if >0 //RESERVED
	BYTE justx;             // IMPMARKJUSTX // Mark justification X
	BYTE justy;             // IMPMARKJUSTY // Mark justification Y
	WORD rep;               // Number of repeat marks (rep+1 is total)
	IMPMARKTYPE type;       // Mark type
	union
		{
		IMPTEXTMARK          t; // type==mtTEXT
		IMPBITMAPMARK        b; // type==mtBITMAP
		IMPDEVBCMARK     devbc; // type==mtDEVBC
		IMPC2OF5MARK     c2of5; // type==mtC2OF5
		IMPC39MARK         c39; // type==mtC39
		IMPC93MARK         c93; // type==mtC93
		IMPC128MARK       c128; // type==mtC128
		IMPCUPCEANMARK cupcean; // type==mtCUPCEAN
		IMPCDMMARK         cdm; // type==mtCDM
		IMPWINFONTMARK winfont; // type==mtWINFONT
		IMPWMTRXMARK     wmtrx; // type==mtWMTRX
		IMPCQRMARK         cqr; // type==mtCQR
		IMPPATCHMARK     patch; // type=mtPATCH
		IMPMULTILINEMARK    ml; // type==mtMULTILINE
		IMPCURESTEP       cure; // type==mtCURE
		IMPCDEVMARK       cdev; // type==mtCDEV
		IMPDEVSYMMARK   devsym; // type==mtDEVSYM
		IMPUIDMARK         uid; // type==mtUID                            //---------------------------Linghai-6-19-09----
		};
	IMPMARK();     // Fill in members with defaults
	~IMPMARK(); 
	void initByType();
	
	BOOL read(HANDLE fh,WORD ver);
	BOOL write(HANDLE fh);
private:
	BOOL readnative(HANDLE fh, WORD ver);
	BOOL translate(HANDLE fh, WORD ver);
	BOOL translateVers343Thru368(HANDLE fh, WORD ver);
	BOOL translateVer341(HANDLE fh,BOOL v339);
	BOOL translateVer338(HANDLE fh);
	BOOL translateOriginalVersion(HANDLE fh);
	// Version 3.30 and original version are the same
};

class IMPMARKS
	{
public:
	WORD count;      // number of marks
	WORD capac;      // current capacity
	IMPMARK * mark; // count IMPMARK structs
	IMPMARKS(const IMPMARKS & src);
	IMPMARKS& operator= (const IMPMARKS & src);
	IMPMARKS();
	IMPMARKS(int initCapac);
	~IMPMARKS();
	void free();
	BOOL realloc(int ncount);
	BOOL write(HANDLE hFile);
	BOOL read(HANDLE fh, WORD ver, WORD count);
private:
	BOOL readnative(HANDLE fh, WORD ver, WORD count);
	BOOL translate(HANDLE fh, WORD ver, WORD count);
	};

struct IMPFONTS
	{
public:
	WORD count;      // number of fonts
	WORD capac;      // current capacity
	IJFONTNAME * name; 
	IMPFONTS(const IMPFONTS & src);
	IMPFONTS& operator= (const IMPFONTS & src);
	IMPFONTS();
	IMPFONTS(int initCapac);
	~IMPFONTS();
	void free();
	BOOL realloc(int ncount);
	BOOL write(HANDLE hFile);
	BOOL read(HANDLE fh, WORD ver, WORD count);
private:
	BOOL readnative(HANDLE fh, WORD ver, WORD count);
	BOOL translate(HANDLE fh, WORD ver, WORD count);
	};

struct IMCORRS
	{
	WORD ncorrs;
	IMCORR imc[IMPMAXMARKS]; // IMCORECTIONs
	IMCORRS(IMCORRS *src);
	IMCORRS();
	IMCORR* find(int markid);
	IMCORR* getempty(void);
	void init(void);
	BOOL read(HANDLE fh);
	};

class IMP

	{
public:
	IMPHEADER *pheader; // The header
	IMPUDHAS udhas;   // THe user defined header attributes
	IMPVARS vars;     // Instance vars
	IMPVARS gvars;    // Global vars
	IMPVALS gvals;    // Global var values
	IMPMARKS marks;   // The marks
	IMPFONTS fonts;   // The fonts table
	BOOL changed;  // Changes not saved?
	//OFSTRUCT ofs;  // The file
	char fn[MAX_PATH]; // The file
	AESETUP *aesetup; // Per part alignment setup or NULL if none.
	IMCORRS imcorrs;   // The IMCORRECTIONs table
	IMP();
	};

/***************************************************************************
	PUBLIC FUNCTIONS
***************************************************************************/
BOOL IMPLoad(PIMP pimp);
BOOL IMPSave(PIMP pimp);
BOOL IMPNew(PIMP pimp);
void IMPFree(PIMP pimp);
BOOL IMPValid(PIMP pimp);
void IMPVarToTbuf(IMPVAR *var);
void IMPVarValToTbuf(IMPVAR *pvar,IMPVAL *pval);
void IMPVarNValToTbuf(IMPVAR *pvar,int n,IMPVAL *pval,bool occupied);
void IMPVarValToIni(IMPVAR *pvar,IMPVAL *pval,LPSTR sect,LPSTR ini);
void IMPVarNValToIni(IMPVAR *pvar,int n,IMPVAL *pval,LPSTR sect,LPSTR ini);
void IMPVarValFromIni(IMPVAR *pvar,IMPVAL *pval,LPSTR sect,LPSTR ini);
void IMPVarNValFromIni(IMPVAR *pvar,int n,IMPVAL *pval,LPSTR sect,LPSTR ini);
void IMPMarkToTbuf(IMPMARK *pm,IMPFONTS *pf);
void IMPHeaderAttributeToTbuf(IMPHEADER *ph,IMPUDHAS *pudha,DWORD i);
BOOL IMPHAIsHidden(DWORD i);
BOOL IMPInitVals(IMPVALS *pvals,IMPVARS *pvars);
BOOL IMPInitVal(IMPVAL *pval,IMPVAR *pvar);
BOOL IMPNextVals(IMPVALS *vals,IMPVARS *vars,BOOL showerror);
BOOL IMPNextSerialVal(IMPVAL *pval,IMPVAR *pvar);
BOOL IMPNextBasedVal(IMPVAL *pval,IMPVAR *pvar);
void IMPLoadBases(void);
void IMPFreeBases(void);
BOOL IMPValidateBasedVal(IMPVAL *pval,IMPVAR *pvar);
void IMPAddBasesToCB(HWND hctrl,int minlen,int maxlen);
PIMSTEP IMPCompileMarks(PIMP pimp,IMPVALS *pvals,IMXYMODE xymode);
PIMSTEP IMPCompileCureSteps(PIMP pimp,IMXYMODE xymode,int nesti);
int IMPExpandTextToBuf(LPSTR buf,int bufsize,LPSTR text,
	IMPVARS *vars,IMPVALS *vals,IMPVARS *gvars,IMPVALS *gvals,
	IMPHEADER *ph,IMPUDHAS *pudhas,BOOL showerr);
BOOL IMPExpandTextToTbuf(LPSTR text,IMPVARS *vars,IMPVALS *vals,
	IMPVARS *gvars,IMPVALS *gvals,IMPHEADER *ph,IMPUDHAS *pudhas);
void IMPPurgeFonts(PIMP pimp);
WORD ValToBuf(IMPVALS *vals,WORD i,IMPVARS *vars,LPSTR buf);
WORD HAToBuf(IMPHEADER *ph,IMPUDHAS *pudha,DWORD i,LPSTR buf);
int IMPLoadDesc(char *fname,char *fdesc);
BOOL IMPSigMatch(LPSTR sig);
void IMPClearLBMP(void);
LPCSTR IMPGetBasedSet(LPSTR basename);
int _IMPGetUDHAInt(IMPUDHAS *pu,LPSTR key,int def);
int IMPGetUDHAInt(PIMP pimp,LPSTR key,int def);
int IMPCountMarks(PIMP pimp);
int IMPCountCureSteps(PIMP pimp);
HBITMAP IMPCompileRaster(IMPMARK *m,
	IMPVARS *pvars,IMPVALS *pvals,IMPVARS *pgvars,IMPVALS *pgvals,
	IMPHEADER *pheader,IMPUDHAS *pudhas,
	int *bfi,double *xres,double *yres,WORD *ms,BOOL *neg,BOOL showerr);

BOOL IMPDeviceMarkIJFMTINFO(IMP* pimp,IMPMARK *m,IJFMTINFO *fmti);
BOOL IMPIsNativeVersion(WORD ver);




/***************************************************************************
	PUBLIC VARIABLES
***************************************************************************/
// Basic Header Attribute names
extern char *IMPBHANames[IMPNBHAS];
extern char *IMPSysVarNames[IMPNSYSVARS];
//extern char *IMPUIDVarNames[IMPUIDVARS];               //--------------Linghai-4-20-09----

#endif // _IMP_H
