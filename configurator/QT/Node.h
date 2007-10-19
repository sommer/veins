#include "ui_node.h"
#include "../miximConfiguratorCommon.h"

class Node: public QDialog, private Ui::Node {
	Q_OBJECT

	public:
		Node(QDialog *parent = NULL);
		unsigned getCount(void);
		QString getType(void);
		QString getNetworkLayer(void);
		QString getApplicationName(void);

	private slots:
		void on_addNICButton_clicked(void);
		void on_regularGridRB_toggled(bool);
		void on_randomRB_toggled(bool);
		void on_squareCB_toggled(bool);
		void on_threeDCB_toggled(bool);
		void on_dimXTB_textChanged(const QString&);
		void on_fixedAnchorCB_toggled(bool);
		void on_nextButton_clicked(void);
		void on_cancelButton_clicked(void);
};

