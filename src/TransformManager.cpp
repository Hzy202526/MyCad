#include "TransformManager.h"
#include "Document.h"
#include "Modeling.h"
#include <QMessageBox>
#include <algorithm>

TransformManager::TransformManager(QObject* parent)
    : QObject(parent)
    , m_document(nullptr)
{
}

bool TransformManager::booleanUnion(const QList<TopoDS_Shape>& shapes, const QList<Handle(AIS_Shape)>& aisShapes)
{
    if (shapes.size() < 2 || m_document == nullptr) {
        return false;
    }
    
    TopoDS_Shape result = shapes[0];
    for (int i = 1; i < shapes.size(); ++i) {
        result = Modeling::booleanUnion(result, shapes[i]);
    }
    
    if (!result.IsNull()) {
        // 移除原始形状（从后往前移除，避免索引变化）
        QList<int> indicesToRemove;
        for (const auto& aisShape : aisShapes) {
            int index = m_document->findShapeIndex(aisShape);
            if (index >= 0) {
                indicesToRemove.append(index);
            }
        }
        // 排序并去重，从大到小排序以便从后往前移除
        std::sort(indicesToRemove.begin(), indicesToRemove.end(), std::greater<int>());
        indicesToRemove.erase(std::unique(indicesToRemove.begin(), indicesToRemove.end()), indicesToRemove.end());
        
        for (int index : indicesToRemove) {
            m_document->removeShape(index);
        }
        
        // 添加结果
        m_document->addShape(result, "Union");
        emit transformCompleted();
        return true;
    }
    
    return false;
}

bool TransformManager::booleanCut(const QList<TopoDS_Shape>& shapes, const QList<Handle(AIS_Shape)>& aisShapes)
{
    if (shapes.size() < 2 || m_document == nullptr) {
        return false;
    }
    
    TopoDS_Shape result = shapes[0];
    for (int i = 1; i < shapes.size(); ++i) {
        result = Modeling::booleanCut(result, shapes[i]);
    }
    
    if (!result.IsNull()) {
        // 移除原始形状
        QList<int> indicesToRemove;
        for (const auto& aisShape : aisShapes) {
            int index = m_document->findShapeIndex(aisShape);
            if (index >= 0) {
                indicesToRemove.append(index);
            }
        }
        std::sort(indicesToRemove.begin(), indicesToRemove.end(), std::greater<int>());
        indicesToRemove.erase(std::unique(indicesToRemove.begin(), indicesToRemove.end()), indicesToRemove.end());
        
        for (int index : indicesToRemove) {
            m_document->removeShape(index);
        }
        
        m_document->addShape(result, "Cut");
        emit transformCompleted();
        return true;
    }
    
    return false;
}

bool TransformManager::booleanIntersect(const QList<TopoDS_Shape>& shapes, const QList<Handle(AIS_Shape)>& aisShapes)
{
    if (shapes.size() < 2 || m_document == nullptr) {
        return false;
    }
    
    TopoDS_Shape result = shapes[0];
    for (int i = 1; i < shapes.size(); ++i) {
        result = Modeling::booleanIntersect(result, shapes[i]);
    }
    
    if (!result.IsNull()) {
        // 移除原始形状
        QList<int> indicesToRemove;
        for (const auto& aisShape : aisShapes) {
            int index = m_document->findShapeIndex(aisShape);
            if (index >= 0) {
                indicesToRemove.append(index);
            }
        }
        std::sort(indicesToRemove.begin(), indicesToRemove.end(), std::greater<int>());
        indicesToRemove.erase(std::unique(indicesToRemove.begin(), indicesToRemove.end()), indicesToRemove.end());
        
        for (int index : indicesToRemove) {
            m_document->removeShape(index);
        }
        
        m_document->addShape(result, "Intersect");
        emit transformCompleted();
        return true;
    }
    
    return false;
}

bool TransformManager::translate(const QList<TopoDS_Shape>& shapes, const gp_Vec& vec)
{
    if (shapes.isEmpty() || m_document == nullptr) {
        return false;
    }
    
    bool success = false;
    for (const auto& shape : shapes) {
        TopoDS_Shape translated = Modeling::translate(shape, vec);
        if (!translated.IsNull()) {
            m_document->addShape(translated, "Translated");
            success = true;
        }
    }
    
    if (success) {
        emit transformCompleted();
    }
    
    return success;
}

bool TransformManager::rotate(const QList<TopoDS_Shape>& shapes, const gp_Ax1& axis, double angle)
{
    if (shapes.isEmpty() || m_document == nullptr) {
        return false;
    }
    
    bool success = false;
    for (const auto& shape : shapes) {
        TopoDS_Shape rotated = Modeling::rotate(shape, axis, angle);
        if (!rotated.IsNull()) {
            m_document->addShape(rotated, "Rotated");
            success = true;
        }
    }
    
    if (success) {
        emit transformCompleted();
    }
    
    return success;
}

bool TransformManager::mirror(const QList<TopoDS_Shape>& shapes, const gp_Ax2& axis)
{
    if (shapes.isEmpty() || m_document == nullptr) {
        return false;
    }
    
    bool success = false;
    for (const auto& shape : shapes) {
        TopoDS_Shape mirrored = Modeling::mirror(shape, axis);
        if (!mirrored.IsNull()) {
            m_document->addShape(mirrored, "Mirrored");
            success = true;
        }
    }
    
    if (success) {
        emit transformCompleted();
    }
    
    return success;
}

bool TransformManager::linearArray(const TopoDS_Shape& shape, const gp_Vec& direction, int count, double spacing)
{
    if (shape.IsNull() || m_document == nullptr) {
        return false;
    }
    
    QList<TopoDS_Shape> array = Modeling::linearArray(shape, direction, count, spacing);
    
    for (const auto& s : array) {
        m_document->addShape(s, "ArrayItem");
    }
    
    emit transformCompleted();
    return true;
}

bool TransformManager::circularArray(const TopoDS_Shape& shape, const gp_Pnt& center, const gp_Dir& axis, int count, double angle)
{
    if (shape.IsNull() || m_document == nullptr) {
        return false;
    }
    
    QList<TopoDS_Shape> array = Modeling::circularArray(shape, center, axis, count, angle);
    
    for (const auto& s : array) {
        m_document->addShape(s, "ArrayItem");
    }
    
    emit transformCompleted();
    return true;
}

