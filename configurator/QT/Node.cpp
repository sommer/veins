#include "Node.h"

QWizardPage* Node::firstPage() {
	QWizardPage *firstPage;
    QLabel *textLabel5;
    QPushButton *deleteNICButton;
    QLabel *textLabel1;
    QLabel *textLabel2;
    QPushButton *editNICButton;
    QFrame *line5;
    QLabel *textLabel1_2;
    QLabel *textLabel1_3;
    QPushButton *addNICButton;

    QGridLayout* layout = new QGridLayout();
	
	firstPage = new QWizardPage(this);
	firstPage->setGeometry(QRect(0, 0, 527, 416));
    firstPage->setTitle("Internal node setup");
    firstPage->setSubTitle("Choose components, application, etc");

    textLabel5 = new QLabel(firstPage);
    textLabel5->setText("Application name:");
    textLabel5->setWordWrap(false);
    layout->addWidget(textLabel5, 0, 0);

    applicationNameCB = new QComboBox(firstPage);
    applicationNameCB->setEditable(false);
    applicationNameCB->setObjectName("applicationNameCB");
    layout->addWidget(applicationNameCB, 0, 1);

    appOptionButton = new QPushButton(firstPage);
    appOptionButton->setEnabled(false);
	appOptionButton->setText("Options...");
	appOptionButton->setObjectName(QString::fromUtf8("appOptionButton"));
	layout->addWidget(appOptionButton, 0, 2);

    textLabel1 = new QLabel(firstPage);
    textLabel1->setWordWrap(false);
    textLabel1->setText("Node type:");
    layout->addWidget(textLabel1);
    
    nodeTypeCB = new QComboBox(firstPage);
    nodeTypeCB->setObjectName("nodeTypeCB");
	layout->addWidget(nodeTypeCB);
	
    nodeOptionButton = new QPushButton(firstPage);
    nodeOptionButton->setEnabled(false);
	nodeOptionButton->setText("Options...");
	nodeOptionButton->setObjectName(QString::fromUtf8("nodeOptionButton"));
	layout->addWidget(nodeOptionButton);
	
    textLabel2 = new QLabel(firstPage);
    textLabel2->setWordWrap(false);
    textLabel2->setText("Routing layer:");
    layout->addWidget(textLabel2);
    
    networkLayerCB = new QComboBox(firstPage);
	layout->addWidget(networkLayerCB);
	
	routingOptionButton = new QPushButton(firstPage);
    routingOptionButton->setEnabled(false);
	routingOptionButton->setText("Options...");
	routingOptionButton->setObjectName(QString::fromUtf8("routingOptionButton"));
	layout->addWidget(routingOptionButton);

    textLabel1_2 = new QLabel(firstPage);
    textLabel1_2->setWordWrap(false);
    textLabel1_2->setText("Amount of nodes:");
    layout->addWidget(textLabel1_2);

    nodeCountSB = new QSpinBox(firstPage);
    nodeCountSB->setMinimum(1);
    nodeCountSB->setMaximum(65535);
    layout->addWidget(nodeCountSB);
	
    line5 = new QFrame(firstPage);
    line5->setFrameShape(QFrame::HLine);
    line5->setFrameShadow(QFrame::Sunken);
    layout->addWidget(line5, 4, 0, 1, 3);
    
    textLabel1_3 = new QLabel(firstPage);
    textLabel1_3->setWordWrap(false);
    textLabel1_3->setText("Network interfaces:");    
    layout->addWidget(textLabel1_3);
    
    nicTable = new QTableView(firstPage);
    nicTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    layout->addWidget(nicTable, 7, 0, 1, 3);

    addNICButton = new QPushButton(firstPage);
	addNICButton->setText("Add");
	addNICButton->setObjectName(QString::fromUtf8("addNICButton"));
	layout->addWidget(addNICButton);

    editNICButton = new QPushButton(firstPage);
	editNICButton->setText("Edit");
	editNICButton->setObjectName(QString::fromUtf8("editNICButton"));
	layout->addWidget(editNICButton);

    deleteNICButton = new QPushButton(firstPage);
    deleteNICButton->setText("Remove");
    deleteNICButton->setObjectName(QString::fromUtf8("deleteNICButton"));
    layout->addWidget(deleteNICButton);
    
	firstPage->setLayout(layout);
	
    return firstPage;
}

QWizardPage* Node::secondPage() {
    QLabel *textLabel6_2_3_2;
    QCheckBox *threeDCB;
    QCheckBox *squareCB;
    QLabel *textLabel6_2_2;
    QLabel *textLabel4_2;
    QLabel *textLabel5_2_3;
    QLabel *textLabel5_2_2;
    QLabel *textLabel6_2_3;
    QFrame *line5_2_2;
    QLabel *textLabel6_2;
    QLabel *textLabel5_2;
    QFrame *line5_2;
    QLabel *label;
    QLabel *label_2;

	QWizardPage *secondPage = new QWizardPage(this);
	QVBoxLayout* layout = new QVBoxLayout();
    QGridLayout* llayout = new QGridLayout();
    QGridLayout* dimlayout = new QGridLayout();
    QHBoxLayout* sinklayout = new QHBoxLayout();
    
    label = new QLabel(secondPage);
    label->setPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/grid.xpm")));
    label->setScaledContents(false);
    llayout->addWidget(label, 0, 0);
    
    label_2 = new QLabel(secondPage);
    label_2->setPixmap(QPixmap(QString::fromUtf8(":/new/prefix1/random.xpm")));
    llayout->addWidget(label_2, 0, 1);
    
    regularGridRB = new QRadioButton(secondPage);
    regularGridRB->setObjectName(QString::fromUtf8("regularGridRB"));
    regularGridRB->setChecked(true);
    llayout->addWidget(regularGridRB);

    randomRB = new QRadioButton(secondPage);
    randomRB->setObjectName(QString::fromUtf8("randomRB"));
    llayout->addWidget(randomRB);
    layout->addLayout(llayout);
        
    line5_2 = new QFrame(secondPage);
    line5_2->setFrameShape(QFrame::HLine);
    line5_2->setFrameShadow(QFrame::Sunken);
    layout->addWidget(line5_2);
    
    textLabel4_2 = new QLabel(secondPage);
    textLabel4_2->setWordWrap(false);
    textLabel4_2->setText(QApplication::translate("Node", "Dimensions:", 0, QApplication::UnicodeUTF8));
    layout->addWidget(textLabel4_2);
    
    textLabel6_2_2 = new QLabel(secondPage);
    textLabel6_2_2->setWordWrap(false);
    textLabel6_2_2->setText(QApplication::translate("Node", "X", 0, QApplication::UnicodeUTF8));
    dimlayout->addWidget(textLabel6_2_2);

    dimXTB = new QLineEdit(secondPage);
    dimXTB->setObjectName(QString::fromUtf8("dimXTB"));
    dimlayout->addWidget(dimXTB, 0, 1);
    
    textLabel6_2_3 = new QLabel(secondPage);
    textLabel6_2_3->setWordWrap(false);
    textLabel6_2_3->setText(QApplication::translate("Node", "Y", 0, QApplication::UnicodeUTF8));
    dimlayout->addWidget(textLabel6_2_3);
    
    dimYTB = new QLineEdit(secondPage);
    dimYTB->setObjectName(QString::fromUtf8("dimYTB"));
    dimYTB->setEnabled(false);
    dimYTB->setReadOnly(false);
    dimlayout->addWidget(dimYTB, 1, 1);
    
    squareCB = new QCheckBox(secondPage);
    squareCB->setChecked(true);
    squareCB->setObjectName("squareCB");
    dimlayout->addWidget(squareCB, 1, 2);
        
    textLabel6_2_3_2 = new QLabel(secondPage);
    textLabel6_2_3_2->setWordWrap(false);
    textLabel6_2_3_2->setText(QApplication::translate("Node", "Z", 0, QApplication::UnicodeUTF8));
    dimlayout->addWidget(textLabel6_2_3_2);
    
    dimZTB = new QLineEdit(secondPage);
    dimZTB->setObjectName(QString::fromUtf8("dimZTB"));
    dimZTB->setEnabled(false);
    dimZTB->setReadOnly(false);
    dimlayout->addWidget(dimZTB);
    
    threeDCB = new QCheckBox(secondPage);
    threeDCB->setObjectName(QString::fromUtf8("threeDCB"));
    dimlayout->addWidget(threeDCB);
    layout->addLayout(dimlayout);
        
    line5_2_2 = new QFrame(secondPage);
    line5_2_2->setFrameShape(QFrame::HLine);
    line5_2_2->setFrameShadow(QFrame::Sunken);
    layout->addWidget(line5_2_2);
    
    fixedAnchorCB = new QCheckBox(secondPage);
    fixedAnchorCB->setObjectName(QString::fromUtf8("fixedAnchorCB"));
    sinklayout->addWidget(fixedAnchorCB);
        
    textLabel6_2 = new QLabel(secondPage);
    textLabel6_2->setWordWrap(false);
    textLabel6_2->setText(QApplication::translate("Node", "ID", 0, QApplication::UnicodeUTF8));
    sinklayout->addWidget(textLabel6_2);
    
    sinkIDTB = new QLineEdit(secondPage);
    sinkIDTB->setObjectName(QString::fromUtf8("sinkIDTB"));
    sinkIDTB->setEnabled(false);
    sinkIDTB->setMaximumSize(QSize(40, 32767));
    sinkIDTB->setReadOnly(false);
    sinklayout->addWidget(sinkIDTB);
        
    textLabel5_2 = new QLabel(secondPage);
    textLabel5_2->setWordWrap(false);
    textLabel5_2->setText(QApplication::translate("Node", "x", 0, QApplication::UnicodeUTF8));
    sinklayout->addWidget(textLabel5_2);
    
    sinkXTB = new QLineEdit(secondPage);
    sinkXTB->setObjectName(QString::fromUtf8("sinkXTB"));
    sinkXTB->setEnabled(false);
    sinkXTB->setMaximumSize(QSize(40, 32767));
    sinkXTB->setReadOnly(false);
    sinklayout->addWidget(sinkXTB);
        
    textLabel5_2_3 = new QLabel(secondPage);
    textLabel5_2_3->setWordWrap(false);
    textLabel5_2_3->setText(QApplication::translate("Node", "y", 0, QApplication::UnicodeUTF8));
    sinklayout->addWidget(textLabel5_2_3);
    
    sinkYTB = new QLineEdit(secondPage);
    sinkYTB->setObjectName(QString::fromUtf8("sinkYTB"));
    sinkYTB->setEnabled(false);
    sinkYTB->setMaximumSize(QSize(40, 32767));
    sinkYTB->setReadOnly(false);
    sinklayout->addWidget(sinkYTB);
        
    textLabel5_2_2 = new QLabel(secondPage);
    textLabel5_2_2->setWordWrap(false);
    textLabel5_2_2->setText(QApplication::translate("Node", "z", 0, QApplication::UnicodeUTF8));
    sinklayout->addWidget(textLabel5_2_2);
    
    sinkZTB = new QLineEdit(secondPage);
    sinkZTB->setObjectName(QString::fromUtf8("sinkZTB"));
    sinkZTB->setEnabled(false);
    sinkZTB->setMaximumSize(QSize(40, 32767));
    sinkZTB->setReadOnly(false);
    sinklayout->addWidget(sinkZTB);
    layout->addLayout(sinklayout);
    
    secondPage->setTitle(QApplication::translate("Node", "Node layout", 0, QApplication::UnicodeUTF8));
    secondPage->setSubTitle(QApplication::translate("Node", "Specify the placement of the nodes", 0, QApplication::UnicodeUTF8));
    sinkIDTB->setText(QApplication::translate("Node", "0", 0, QApplication::UnicodeUTF8));
    sinkYTB->setText(QApplication::translate("Node", "0", 0, QApplication::UnicodeUTF8));
    dimYTB->setText(QApplication::translate("Node", "100", 0, QApplication::UnicodeUTF8));
    threeDCB->setText(QApplication::translate("Node", "3-D", 0, QApplication::UnicodeUTF8));
    regularGridRB->setText(QApplication::translate("Node", "Regular grid", 0, QApplication::UnicodeUTF8));
    squareCB->setText(QApplication::translate("Node", "square", 0, QApplication::UnicodeUTF8));
    sinkXTB->setText(QApplication::translate("Node", "0", 0, QApplication::UnicodeUTF8));
    sinkZTB->setText(QApplication::translate("Node", "0", 0, QApplication::UnicodeUTF8));
    randomRB->setText(QApplication::translate("Node", "Random", 0, QApplication::UnicodeUTF8));
    dimXTB->setText(QApplication::translate("Node", "100", 0, QApplication::UnicodeUTF8));
    fixedAnchorCB->setText(QApplication::translate("Node", "Fixed position for sink:", 0, QApplication::UnicodeUTF8));
    dimZTB->setText(QApplication::translate("Node", "-", 0, QApplication::UnicodeUTF8));
    
    secondPage->setLayout(layout);
    
	return secondPage;
}

Node::Node(QWidget *parent): QWizard(parent) {
	stopToggling = false;
	threeD = false;
	square = true;

	addPage(firstPage());
	addPage(secondPage());
	setupUi(this);

	nodeModules = findModules(NODE);
	currentNodeMod = nodeModules;
	
	appModules = findModules(APPLICATION);
	currentAppMod = appModules;
	
	macModules = findModules(MAC);
	phyModules = findModules(PHY);
	networkModules = findModules(NETWORK);
	
	if (nodeModules != NULL) {
		Module* it = nodeModules;
		while (it) {
			nodeTypeCB->insertItem(0, QString(it->name));
			it = it->next;
		}
	}
	
	if (networkModules != NULL) {
		Module* it = networkModules;
		while (it) {
			networkLayerCB->insertItem(0, QString(it->name));
			it = it->next;
		}
	}
	
	if (appModules != NULL) {
		Module* it = appModules;
		while (it) {
			applicationNameCB->insertItem(0, QString(it->name));
			it = it->next;
		}
	}
	
	if (macModules != NULL) {
		Module* it = macModules;
		while (it) {
			macNames.append(it->name);
			it = it->next;
		}
	}
	
	
	if (phyModules != NULL) {
		Module* it = phyModules;
		while (it) {
			phyNames.append(it->name);
			it = it->next;
		}
	}
	
	// these are parameters for the 3 base classes
	nodeOptionButton->setEnabled(currentNodeMod->parameters != NULL);
	appOptionButton->setEnabled(appModules->parameters != NULL);
		
	if (networkModules->parameters != NULL)
		routingOptionButton->setEnabled(true);
}

void Node::setModel(QStandardItemModel* newModel) {
	model = newModel;
	nicTable->setModel(model);
}

QStandardItemModel* Node::getModel() {
	return model;
}

unsigned Node::getCount() {
	return nodeCountSB->value();
}

QString Node::getType() {
	return nodeTypeCB->currentText();
}

QString Node::getNetworkLayer() {
	return networkLayerCB->currentText();
}

QString Node::getApplicationName() {
	return applicationNameCB->currentText();
}

void Node::on_addNICButton_clicked() {
	//nicTable->insertRow(0);
	//nicTable->setItem(0, 0, new QTableWidgetItem("new"));
	//nicTable->setItem(0, 1, new QComboBox());
//	nicTable->setItem(0, 2, new Q3ComboTableItem(nicTable, phyNames));
}

void Node::on_deleteNICButton_clicked() {
	QModelIndexList indexes = nicTable->selectionModel()->selectedIndexes();
	QModelIndex index;
	int lastRow =-1;

	foreach(index, indexes) {
		if (index.row() != lastRow) {
			printf("Selected: row %d column %d\n", index.row(), index.column());
			model->removeRows(index.row(), 1, index.parent());
			lastRow = index.row();	// as the selection model is rows only; we can do this little trick
		}
	}
}

void Node::on_editNICButton_clicked() {
	// TODO
}

void Node::on_regularGridRB_toggled( bool ) {
	if (stopToggling)
		stopToggling = false;
	else {
		stopToggling =true;
		randomRB->toggle();
	}
}

void Node::on_randomRB_toggled( bool ) {
	if (stopToggling)
		stopToggling = false;
	else {
		stopToggling =true;
		regularGridRB->toggle();
	}
}

void Node::on_squareCB_stateChanged(int newState) {
	square = newState != 0;
	dimYTB->setEnabled(!square);
	if (threeD)
		dimZTB->setEnabled(!square);
	if (square) {
		dimYTB->setText(dimXTB->text());
		if (threeD)
			dimZTB->setText(dimXTB->text());
	}
}

void Node::on_threeDCB_stateChanged(int newState) {
	threeD = newState != 0;
	dimZTB->setEnabled(threeD && !square);
	if (threeD)
		dimZTB->setText(dimXTB->text());
	else
		dimZTB->setText("-");
}

void Node::on_dimXTB_textChanged( const QString & ) {
	if (square) {
		dimYTB->setText(dimXTB->text());
		if (threeD)
			dimZTB->setText(dimXTB->text());
	}
}

void Node::on_fixedAnchorCB_stateChanged(int enable) {
	sinkIDTB->setEnabled(enable);
	sinkXTB->setEnabled(enable);
	sinkYTB->setEnabled(enable);
	sinkZTB->setEnabled(enable);
}

void Node::on_nodeOptionButton_clicked() {
	OptionDialog dialog(currentNodeMod);
	dialog.exec();
}

void Node::on_routingOptionButton_clicked() {
	OptionDialog dialog(networkModules);
	dialog.exec();
}

void Node::on_appOptionButton_clicked() {
	OptionDialog dialog(appModules);
	dialog.exec();
}

void Node::on_nodeTypeCB_activated(const QString &text) {
	currentNodeMod = getModuleByName(nodeModules, text.toAscii().data());
	nodeOptionButton->setEnabled(currentNodeMod->parameters != NULL);
}

void Node::on_applicationNameCB_activated(const QString &text) {
	currentAppMod = getModuleByName(appModules, text.toAscii().data());
	appOptionButton->setEnabled(currentAppMod->parameters != NULL);
}
