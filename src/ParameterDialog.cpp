#include "ParameterDialog.h"
#include <QFormLayout>

ParameterDialog::ParameterDialog(const QString& title, QWidget* parent)
    : QDialog(parent)
    , m_mainLayout(nullptr)
    , m_formLayout(nullptr)
{
    setWindowTitle(title);
    setModal(true);
    setMinimumWidth(300);
    
    m_mainLayout = new QVBoxLayout(this);
    
    // 创建表单布局用于参数输入
    m_formLayout = new QFormLayout();
    m_mainLayout->addLayout(m_formLayout);
    
    // 创建按钮框
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, 
        Qt::Horizontal, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    
    m_mainLayout->addWidget(buttonBox);
}

void ParameterDialog::addParameter(const QString& label, double defaultValue, 
                                   double minValue, double maxValue, int decimals)
{
    QLabel* labelWidget = new QLabel(label + ":", this);
    
    QDoubleSpinBox* spinBox = new QDoubleSpinBox(this);
    spinBox->setMinimum(minValue);
    spinBox->setMaximum(maxValue);
    spinBox->setValue(defaultValue);
    spinBox->setDecimals(decimals);
    spinBox->setSingleStep(1.0);
    spinBox->setMinimumWidth(150);
    
    // 使用表单布局添加行
    if (m_formLayout) {
        m_formLayout->addRow(labelWidget, spinBox);
    }
    
    m_labels.append(labelWidget);
    m_spinBoxes.append(spinBox);
}

double ParameterDialog::getParameter(int index) const
{
    if (index >= 0 && index < m_spinBoxes.size()) {
        return m_spinBoxes[index]->value();
    }
    return 0.0;
}

QList<double> ParameterDialog::getAllParameters() const
{
    QList<double> params;
    for (auto* spinBox : m_spinBoxes) {
        params.append(spinBox->value());
    }
    return params;
}

void ParameterDialog::setParameter(int index, double value)
{
    if (index >= 0 && index < m_spinBoxes.size()) {
        m_spinBoxes[index]->setValue(value);
    }
}

