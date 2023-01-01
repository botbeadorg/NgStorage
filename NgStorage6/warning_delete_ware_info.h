// ---------------------------------------------------------------------------

#ifndef warning_delete_ware_infoH
#define warning_delete_ware_infoH
// ---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <FMX.Controls.hpp>
#include <FMX.Forms.hpp>
#include <FMX.Objects.hpp>
#include <FMX.Types.hpp>
#include <FMX.Controls.Presentation.hpp>
#include <FMX.StdCtrls.hpp>

// ---------------------------------------------------------------------------
class TForm4 : public TForm {
__published: // IDE-managed Components
	TText *Text1;
	TButton *Button1;
	TButton *Button2;
	TStyleBook *StyleBook1;

	void __fastcall Button1Click(TObject *Sender);
	void __fastcall Button2Click(TObject *Sender);
	void __fastcall FormCreate(TObject *Sender);

private: // User declarations
public: // User declarations
	__fastcall TForm4(TComponent* Owner);
};

// ---------------------------------------------------------------------------
extern PACKAGE TForm4 *Form4;
// ---------------------------------------------------------------------------
#endif
