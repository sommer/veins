#ifndef __NODE_H__
#define __NODE_H__

#include <QStandardItemModel>
#include <QComboBox>
#include <QSpinBox>
#include <QTableView>
#include <QPushButton>
#include <QWizardPage>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QRadioButton>

#include "ui_node.h"
#include "../miximConfiguratorCommon.h"
#include "optionDialog.h"

class Node: public QWizard, private Ui::Node {
	Q_OBJECT

	public:
		Node(QWidget *parent = NULL);
		unsigned getCount(void);
		QString getType(void);
		QString getNetworkLayer(void);
		QString getApplicationName(void);
		void setModel(QStandardItemModel*);
		QStandardItemModel* getModel(void);

	private:
	    Module* macModules;
		Module* phyModules;
		Module* networkModules;
		
		Module* appModules;
		Module* currentAppMod;
		
		Module* nodeModules;
		Module* currentNodeMod;

		QStringList macNames;
		QStringList phyNames;
		QStandardItemModel* model;
		bool stopToggling;
		bool threeD;
		bool square;
		QComboBox *nodeTypeCB;
		QComboBox *networkLayerCB;
		QComboBox *applicationNameCB;
		QSpinBox *nodeCountSB;
		QTableView *nicTable;
		QPushButton *nodeOptionButton;
		QPushButton *routingOptionButton;
		QPushButton *appOptionButton;
		QRadioButton *randomRB;
	    QRadioButton *regularGridRB;
	    QLineEdit *dimXTB;
	    QLineEdit *dimYTB;
	    QLineEdit *dimZTB;
	    QLineEdit *sinkXTB;
	    QLineEdit *sinkYTB;
	    QLineEdit *sinkZTB;
	    QLineEdit *sinkIDTB;
	    QCheckBox *fixedAnchorCB;
	
		QWizardPage* firstPage(void);
		QWizardPage* secondPage(void);
		
	private slots:
		void on_nodeOptionButton_clicked(void);
		void on_addNICButton_clicked(void);
		void on_deleteNICButton_clicked(void);
		void on_editNICButton_clicked(void);
		void on_regularGridRB_toggled(bool);
		void on_randomRB_toggled(bool);
		void on_squareCB_stateChanged(int);
		void on_threeDCB_stateChanged(int);
		void on_dimXTB_textChanged(const QString&);
		void on_fixedAnchorCB_stateChanged(int);
		void on_routingOptionButton_clicked(void);
		void on_appOptionButton_clicked(void);
		void on_nodeTypeCB_activated(const QString&);
		void on_applicationNameCB_activated(const QString&);
};

#endif
