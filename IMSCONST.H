/******************************************************************************
	Name : IMSCONST.H
	Desc : Constants for IMS4001
******************************************************************************/
#define IMPNSYSVARS            9  // Number of system variables
//#define IMPUIDVARS			  17  // Number of uid variables
#define IMPMAXVARS            32  // Maximum number of variables
#define IMPMAXVALS    IMPMAXVARS  // Maximum number of values
#define IMPMAXMARKS           64  // Maximum number of marks
#define IMPMAXSVVAL    999999999L // Maximum serial number value
#define IMPMAXSVSTEP   999999999L // Maximum serial number step
#define IMPMAXSVREPEAT      9999  // Maximum serial number repeat
#define IMPLVNAME             32  // Length of variable names
#define IMPLKVVAL             48  // Length of keyed variable values
#define IMPLSVVAL              9  // Length of largest serial number
#define IMPLBASEDNAME         20  // Length of longest based name
#define IMPLBASEDVAL          25  // Length of largest based serial number
#define IMPLTEXT             128  // Length of IMPTEXTMARK text
#define IMPLDEVBC             64  // Length of IMPDEVBCMARK text
#define IMPLDEVSYM           128  // Length of IMPDEVSYMMARK text
#define IMPLC39               63  // Length of IMPC39MARK text
#define IMPLC39HRPICTURE      60  // Length of IMPC39MARK hr picture
#define IMPLC2OF5             63  // Length of IMPC2OF5MARK text
#define IMPLC2OF5HRPICTURE    60  // Length of IMPC2OF5MARK hr picture
#define IMPLC128              63  // Length of IMPC128MARK text
#define IMPLC128HRPICTURE     80  // Length of IMPC128MARK hr picture
#define IMPLCUPCEAN           64  // Length of IMPCUPCEANMARK text
#define IMPLCUPCEANHRPICTURE  48  // Length of IMPCUPCEANMARK hr picture
#define IMPLC93               64  // Length of IMPC93MARK text
#define IMPLBMFN             128  // Length of IMPBITMAPMARK bmfn
#define IMPMINMS               1  // Minimum mutistroke factor
#define IMPMAXMS               4  // Maximum mutistroke factor
#define IMPMAXREP              9  // Maximum repeat count
#define IMPNBHAS              22  // n Basic Header Attributes
#define IMPLDESC              63  // Length of header.Desc
#define IMPLAUTHOR            32  // Length of header.Author
#define IMPLFIXTURE            8  // Length of header.Fixture
//RESERVED  IMPLFIXTURE      128  // Length of header.Fixture
#define IMPMAXUDHAS           16  // Max # of User Defined Header Attributes
#define IMPLUDHANAME          31  // Length of UDHA name
#define IMPLUDHAVAL           32  // Length of a UDHA val
#define IMPMAXFONTS           32  // Maximum # of fonts in a file
#define IMPLMLTEXT           128  // Length of IMPMULTILINELMARK text
#define IMPLCDEVTEXT         110  // Length of IMPCOMPOUNDDEVMARK text
#define MAXSIGNITURE          32 // Maximum file signiture length
#define IMPLFNBASE           128 // base filename (no path or ext)
#define IMPLFNEXT              3
#define IMPLFN                12
#define IMPLTIMESUFFIX         5  // Max length of time suffix strings
#define IMPLTIMEOVERRIDE      40
#define IMPLDATEOVERRIDE      40
#define IMPLDATEFORMAT        20
#define IMPLVISPREP           64
#define IMPLCDM              105  // Max length of CDM.text
#define IMPLWINFONT           98  // Max length of WINFONT.text
#define IMPMAXCDEVBSM          8  // Max CDev bitmap sub marks

#define IMPLWMTRX            119  // Max length of WMTRX matrix text (9x12 =108 for FS in binary (!hex))
#define IMPLWMTRXCHTEXT       40  // Max length of WMTRX col heading text (1 char per col)
#define IMPLWMTRXRHTEXT       40  // Max length of WMTRX row heading text (1 char per row)

///zzzwmtrx.flags values
#define IMPFWMTRX_RHR          1 // Row Heading Right (instead of left)
#define IMPFWMTRX_CHB          2 // Col Heading Bottom (instead of top)
#define IMPFWMTRX_ROUND        4 // Round dots (instead of square)

// CQR QR Code
#define IMPLCQR              128   // Max length of QR Code.text
// P2 Encoding
#define IMPCQRNENC           3
typedef enum {IMPCQRENCB=0,IMPCQRENCA,IMPCQRENCN} IMPCQRENC;
// P3 Correction Types
#define IMPCQRNCOR           4
typedef enum {IMPCQRCORL=0,IMPCQRCORM,IMPCQRCORQ=0,IMPCQRCORH=0} IMPCQRCOR;
// P4 Symbol Version
#define IMPCQRNVER 40
#define IMPCQRVERMIN 1
#define IMPCQRVERMAX 40

// UPCEAN mark subtypes
#define IMPNUPCEANSUBTYPES     2  // Number of UPCEAN subtypes
#define UPCA 0
#define EAN8 1

#define CFPP 4 // Coordinate Floating Point Precision

class IMP;
class IMF;
class IMPVALS;

typedef IMP *PIMP;
typedef IMP *LPIMP;
typedef IMF *PIMF;
typedef IMF *LPIMF;

typedef WORD BOOL_16;
