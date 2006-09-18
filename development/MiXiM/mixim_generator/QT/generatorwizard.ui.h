/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/

bool threeD = false;
bool square = true;
bool stopToggling = false;

void generatorWizard::init() {
	setNextEnabled(page(0), false);
	setFinishEnabled(page(1), true);
}

void generatorWizard::RegularGridToggled() {
	if (stopToggling)
		stopToggling = false;
	else {
		stopToggling =true;
		randomRB->toggle();
	}
}


void generatorWizard::randomToggled() {
	if (stopToggling)
		stopToggling = false;
	else {
		stopToggling =true;
		regularGridRB->toggle();
	}
}

void generatorWizard::squareToggled() {
	square = !square;
	dimYTB->setEnabled(!square);
	if (threeD)
		dimZTB->setEnabled(!square);
	if (square) {
		dimYTB->setText(dimXTB->text());
		if (threeD)
			dimZTB->setText(dimXTB->text());
	}
}


void generatorWizard::threeDToggled() {
	threeD = !threeD;
	dimZTB->setEnabled(threeD && !square);
	if (threeD)
		dimZTB->setText(dimXTB->text());
	else
		dimZTB->setText("-");
}

void generatorWizard::dimXChanged() {
	if (square) {
		dimYTB->setText(dimXTB->text());
		if (threeD)
			dimZTB->setText(dimXTB->text());
	}
}


void generatorWizard::fixedAnchorToggle() {
	bool enable = fixedAnchorCB->isChecked();
	sinkIDTB->setEnabled(enable);
	sinkXTB->setEnabled(enable);
	sinkYTB->setEnabled(enable);
	sinkZTB->setEnabled(enable);
}


void generatorWizard::applicationNameChanged() {
	if (applicationNameCB->currentText().isEmpty()) {
		setNextEnabled(page(0), false);
	} else {
		setNextEnabled(page(0), true);
	}
}
