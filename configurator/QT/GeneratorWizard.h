#include "ui_generatorwizard.h"

class GeneratorWizard: public Q3Wizard, private Ui::GeneratorWizard {
	Q_OBJECT

	public:
		GeneratorWizard(Q3Wizard *parent = NULL);

	private:
		void evaluateNextButtonState(void);

	private slots:
		void addNodesButton_clicked(void);
		void on_deleteItemButton_clicked(void);
};

