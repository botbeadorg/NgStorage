// ---------------------------------------------------------------------------

#ifndef task_listH
#define task_listH
// ---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <FMX.Controls.hpp>
#include <FMX.Forms.hpp>
#include <FMX.Controls.Presentation.hpp>
#include <FMX.Grid.hpp>
#include <FMX.Grid.Style.hpp>
#include <FMX.ScrollBox.hpp>
#include <FMX.Types.hpp>
#include <System.Rtti.hpp>
#include <FMX.StdCtrls.hpp>
#include <FMX.Layouts.hpp>
#include <FMX.Ani.hpp>

// ---------------------------------------------------------------------------

class TForm2 : public TForm {
__published: // IDE-managed Components
	TStyleBook *StyleBook1;
	TLayout *Layout1;
	TLayout *Layout2;
	TButton *Button1;
	TStringGrid *StringGrid1;
	TStringColumn *StringColumn1;
	TStringColumn *StringColumn2;
	TStringColumn *StringColumn3;
	TStringColumn *StringColumn4;
	TStringColumn *StringColumn5;
	TStringColumn *StringColumn6;
	TStringColumn *StringColumn7;
	TStringColumn *StringColumn8;
	TStringColumn *StringColumn9;

	void __fastcall FormCreate(TObject *Sender);
	void __fastcall StringGrid1ApplyStyleLookup(TObject *Sender);

private: // User declarations
public: // User declarations
	__fastcall TForm2(TComponent* Owner);

	size_t myrowcount;
};

// ---------------------------------------------------------------------------
extern PACKAGE TForm2 *Form2;
// ---------------------------------------------------------------------------
#endif
