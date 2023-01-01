// ---------------------------------------------------------------------------

#ifndef spec_packsH
#define spec_packsH
// ---------------------------------------------------------------------------

#include <System.Classes.hpp>
#include <FMX.Controls.hpp>
#include <FMX.Forms.hpp>
#include <FMX.ListBox.hpp>
#include <FMX.Types.hpp>
#include <FMX.Colors.hpp>
#include <FMX.Controls.Presentation.hpp>
#include <FMX.StdCtrls.hpp>
#include "common.h"

// ---------------------------------------------------------------------------
class TForm5 : public TForm {
__published: // IDE-managed Components
	TComboBox *ComboBox1;
	TComboBox *ComboBox2;
	TComboBox *ComboBox3;
	TComboBox *ComboBox4;
	TButton *Button1;
	TColorComboBox *ColorComboBox1;
	TStyleBook *StyleBook1;
	TStatusBar *StatusBar1;
	TLabel *Label1;

	void __fastcall FormCreate(TObject *Sender);
	void __fastcall Button1Click(TObject *Sender);
	void __fastcall ComboBox1Change(TObject *Sender);
	void __fastcall FormDestroy(TObject *Sender);

private: // User declarations
public: // User declarations
	__fastcall TForm5(TComponent* Owner);

	rep_spec_pack_with_color *rspwc;
};

// ---------------------------------------------------------------------------
extern PACKAGE TForm5 *Form5;
// ---------------------------------------------------------------------------
#endif
