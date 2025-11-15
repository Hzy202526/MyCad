#ifndef PARAMETERDIALOG_H
#define PARAMETERDIALOG_H

#include <QDialog>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QDialogButtonBox>

class ParameterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ParameterDialog(const QString& title, QWidget* parent = nullptr);
    
    // 添加参数输入字段
    void addParameter(const QString& label, double defaultValue = 0.0, 
                     double minValue = 0.0, double maxValue = 1000.0, 
                     int decimals = 2);
    
    // 获取参数值
    double getParameter(int index) const;
    QList<double> getAllParameters() const;
    
    // 设置参数值
    void setParameter(int index, double value);

private:
    QVBoxLayout* m_mainLayout;
    QFormLayout* m_formLayout;
    QList<QDoubleSpinBox*> m_spinBoxes;
    QList<QLabel*> m_labels;
};

#endif // PARAMETERDIALOG_H

