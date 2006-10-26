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

#include "node.h"

void generatorWizard::addNodesButton_clicked() {
	Node nodeDialog;
	if (nodeDialog.exec() == QDialog::Accepted) {
		objectTable->insertRows(0);
		objectTable->setText(0, 0, QString::number(nodeDialog.getCount()));
		objectTable->setText(0, 1, nodeDialog.getType());
		objectTable->setText(0, 2, nodeDialog.getApplicationName());
		printf("User selected %d nodes type %s with %s running %s\n", nodeDialog.getCount(), nodeDialog.getType().latin1(), nodeDialog.getNetworkLayer().latin1(), nodeDialog.getApplicationName().latin1());
        }
	evaluateNextButtonState();
}

void generatorWizard::deleteItemButton_clicked() {
	int curRow = objectTable->currentRow();
	printf("Row about to be deleted: %d\n", curRow);
	objectTable->removeRow(curRow);
	evaluateNextButtonState();
}

void generatorWizard::evaluateNextButtonState() {
	if (objectTable->numRows() > 0) {
		setFinishEnabled(page(0), true);
	} else {
		setFinishEnabled(page(0), false);
	}
}
