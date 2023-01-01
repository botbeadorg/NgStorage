// ---------------------------------------------------------------------------

#ifndef MatrixFormH
#define MatrixFormH
// ---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <FMX.Controls.hpp>
#include <FMX.Forms.hpp>
#include <FMX.Layouts.hpp>
#include <FMX.Types.hpp>
#include <FMX.Objects.hpp>
#include <FMX.StdCtrls.hpp>
#include <FMX.Controls.Presentation.hpp>
#include <FMX.Grid.hpp>
#include <FMX.Grid.Style.hpp>
#include <FMX.ScrollBox.hpp>
#include <System.Rtti.hpp>
#include <FMX.ListBox.hpp>
#include <FMX.Colors.hpp>
#include <FMX.ListView.Adapters.Base.hpp>
#include <FMX.ListView.Appearances.hpp>
#include <FMX.ListView.hpp>
#include <FMX.ListView.Types.hpp>

typedef DynamicArray<DynamicArray<uintptr_t> >matrix1;

// ---------------------------------------------------------------------------
class TForm3 : public TForm {
__published: // IDE-managed Components
	TLayout *Layout1;
	TRoundRect *RoundRect1;
	TPopup *Popup1;
	TStringGrid *StringGrid1;
	TStringColumn *StringColumn1;
	TStringColumn *StringColumn2;
	TStyleBook *StyleBook1;

	void __fastcall FormShow(TObject *Sender);
	void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
	void __fastcall FormResize(TObject *Sender);
	void __fastcall RoundRect1MouseEnter(TObject *Sender);
	void __fastcall RoundRect1MouseLeave(TObject *Sender);
	void __fastcall Popup1Popup(TObject *Sender);
	void __fastcall Popup1Paint(TObject *Sender, TCanvas *Canvas, const TRectF &ARect);
	void __fastcall Popup1Resized(TObject *Sender);
	void __fastcall FormCreate(TObject *Sender);

private: // User declarations
public: // User declarations
	__fastcall TForm3(TComponent* Owner);
	void __fastcall draw_plan2();
	void __fastcall repaint_plan();
	void __fastcall fake_data();

	matrix1 m;
	bool debut;
	TStringDynArray popup_params;
	float width_popup, height_popup;
};

// ---------------------------------------------------------------------------
extern PACKAGE TForm3 *Form3;
// ---------------------------------------------------------------------------
#endif
