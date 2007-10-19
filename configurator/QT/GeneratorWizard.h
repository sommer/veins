#include "ui_generatorwizard.h"

class GeneratorWizard: public QMainWindow, private Ui::GeneratorWizard {
	Q_OBJECT

	public:
		GeneratorWizard(QMainWindow *parent = NULL);

	private:
		void evaluateButtonsState(void);

	private slots:
		void on_addNodesButton_clicked(void);
		void on_deleteItemButton_clicked(void);
		void on_cancelButton_clicked(void);
		void on_okButton_clicked(void);
};

