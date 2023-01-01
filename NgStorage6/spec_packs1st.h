// ---------------------------------------------------------------------------

#ifndef spec_packs1stH
#define spec_packs1stH
// ---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <FMX.Controls.hpp>
#include <FMX.Forms.hpp>
#include <FMX.Types.hpp>
#include <FMX.Colors.hpp>
#include <FMX.Controls.Presentation.hpp>
#include <FMX.ListBox.hpp>
#include <FMX.StdCtrls.hpp>
#include "common.h"

// ---------------------------------------------------------------------------
class TForm6 : public TForm {
__published: // IDE-managed Components
	TStyleBook *StyleBook1;
	TButton *Button1;
	TComboBox *ComboBox4;
	TComboBox *ComboBox3;
	TComboBox *ComboBox2;
	TComboBox *ComboBox1;
	TColorComboBox *ColorComboBox1;
	TStatusBar *StatusBar1;
	TLabel *Label1;

	void __fastcall FormCreate(TObject *Sender);
	void __fastcall FormDestroy(TObject *Sender);
	void __fastcall Button1Click(TObject *Sender);

private: // User declarations
public: // User declarations
	__fastcall TForm6(TComponent* Owner);

	rep_spec_pack_with_color *rspwc;

	unsigned __fastcall product_small_or_big();
};

// ---------------------------------------------------------------------------
extern PACKAGE TForm6 *Form6;
// ---------------------------------------------------------------------------
#endif
