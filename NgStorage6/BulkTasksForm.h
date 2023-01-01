// ---------------------------------------------------------------------------

#ifndef BulkTasksFormH
#define BulkTasksFormH
// ---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <FMX.Controls.hpp>
#include <FMX.Forms.hpp>
#include <FMX.Types.hpp>
#include <FMX.Controls.Presentation.hpp>
#include <FMX.Grid.hpp>
#include <FMX.Grid.Style.hpp>
#include <FMX.ScrollBox.hpp>
#include <System.Rtti.hpp>
#include <FMX.StdCtrls.hpp>
#include <FMX.Objects.hpp>
#include <FMX.Layouts.hpp>
#include <FMX.Ani.hpp>

// ---------------------------------------------------------------------------
class TForm7 : public TForm {
__published: // IDE-managed Components
	TStyleBook *StyleBook1;
	TScrollBox *ScrollBox1;
	TLayout *Layout1;

	void __fastcall FormCreate(TObject *Sender);
	void __fastcall FormDestroy(TObject *Sender);

private: // User declarations
public: // User declarations
	__fastcall TForm7(TComponent* Owner);

#define N 0
#define S 1
	// TText *txts_produce[2];
	// size_t real_len_txts;
#define A00 0
#define A01 1
#define A03 2
	// TText *txts[2][3];
	// TCheckBox *checkboxes_n[3][30];
	// TCheckBox *checkboxes_s[3][30];

	TAniCalculations *anic;
};

// ---------------------------------------------------------------------------
extern PACKAGE TForm7 *Form7;
// ---------------------------------------------------------------------------
#endif
