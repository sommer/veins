#include "GeneratorWizard.h"
#include "Node.h"

GeneratorWizard::GeneratorWizard(QMainWindow *parent) {
	setupUi(this);
}

void GeneratorWizard::evaluateButtonsState() {
	if (objectTable->rowCount() > 0) {
		okButton->setEnabled(true);
		editItemButton->setEnabled(true);
		deleteItemButton->setEnabled(true);
	} else {
		okButton->setEnabled(false);
		editItemButton->setEnabled(false);
		deleteItemButton->setEnabled(false);
	}
}

void GeneratorWizard::on_addNodesButton_clicked() {
	printf("add button clicked\n");
	Node nodeDialog;
	if (nodeDialog.exec() == QDialog::Accepted) {
		objectTable->insertRow(0);
		objectTable->setItem(0, 0, new QTableWidgetItem(QString::number(nodeDialog.getCount())));
		objectTable->setItem(0, 1, new QTableWidgetItem(nodeDialog.getType()));
		objectTable->setItem(0, 2, new QTableWidgetItem(nodeDialog.getApplicationName()));
		//printf("User selected %d nodes type %s with %s running %s\n", nodeDialog.getCount(), nodeDialog.getType().toLatin1(), nodeDialog.getNetworkLayer().toLatin1(), nodeDialog.getApplicationName().toLatin1());
        }
	evaluateButtonsState();
}

void GeneratorWizard::on_deleteItemButton_clicked() {
	int curRow = objectTable->currentRow();
	printf("Row about to be deleted: %d\n", curRow);
	objectTable->removeRow(curRow);
	evaluateButtonsState();
}

void GeneratorWizard::on_cancelButton_clicked() {
	exit(0);
}

void GeneratorWizard::on_okButton_clicked() {
	close();
}

