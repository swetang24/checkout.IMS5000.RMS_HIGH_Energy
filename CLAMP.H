/******************************************************************************
	NAME : CLAMP
	DESC : Active Clamp Fixture Support.
		A clamping fixture can be controlled by a DIO bit called Clamp.
		It is locked and released at appropriate times in the marking
		and editing cycles and is enabled when the bit Clamp is defined
		and the UDHA Clamp is defined and has a value other than 0.
******************************************************************************/
extern BOOL ClampActive; // Indicates clamp is in use

void ClampInit(void); // Maps bits
void ClampStart(PIMP pimp,IMPUDHAS *pudhas); // Sets ClampActive if UDHA and Bits indicate it.
void ClampLock(void); // Locks the clamp if ClampActive
void ClampRelease(void); // Releases the clamp if ClampActive
void ClampToggle(void);  // Toggles the clamp state
void PC2ClampInit(void); // Maps bits
void PC2ClampRelease(void); // Releases the PC2 io clamp
