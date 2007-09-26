#include "GeneratorWizard.h"
#include "Node.h"

GeneratorWizard::GeneratorWizard(Q3Wizard *parent) {
	setupUi(this);
}

void GeneratorWizard::evaluateNextButtonState() {
	if (objectTable->numRows() > 0) {
		setFinishEnabled(page(0), true);
	} else {
		setFinishEnabled(page(0), false);
	}
}

void GeneratorWizard::addNodesButton_clicked() {
	printf("add button clicked\n");
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

void GeneratorWizard::on_deleteItemButton_clicked() {
	int curRow = objectTable->currentRow();
	printf("Row about to be deleted: %d\n", curRow);
	objectTable->removeRow(curRow);
	evaluateNextButtonState();
}

