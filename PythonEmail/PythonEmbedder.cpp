#if defined(WIN32) || defined(_WIN32)
#if defined(slots)
#undef slots
#endif
#if defined(signals)
#undef signals
#endif
#include <Python.h>
#if defined(QT_VERSION)
#ifndef slots
#define slots Q_SLOTS
#endif
#ifndef signals
#define signals Q_SIGNALS
#endif
#endif
#endif

#include "PythonEmbedder.h"
#include <QDebug>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QThread>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

#ifndef WIN32
#ifndef _WIN32
#include <python3.11/Python.h>
#endif
#endif

static QString g_logFilePath;

static void logToFile(const QString& message) {
    if (g_logFilePath.isEmpty()) {
        QString appPath = QCoreApplication::applicationDirPath();
        g_logFilePath = appPath + "/python_debug.log";
    }
    
    QFile file(g_logFilePath);
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        out << "[" << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz") << "] " 
            << message << "\n";
        file.close();
    }
}

static void crashHandler(const char* message) {
    QString msg = QString("CRASH: %1").arg(message);
    qCritical() << msg;
    logToFile(msg);
}

static QString PyUnicodeToQString(PyObject* obj) {
    if (!obj) return QString();
    
    PyObject* unicode = PyObject_Str(obj);
    if (!unicode) return QString();
    
    PyObject* bytes = PyUnicode_AsEncodedString(unicode, "utf-8", "strict");
    Py_DECREF(unicode);
    
    if (!bytes) return QString();
    
    const char* cstr = PyBytes_AsString(bytes);
    QString result = cstr ? QString::fromUtf8(cstr) : QString();
    Py_DECREF(bytes);
    
    return result;
}

PythonEmbedder::PythonEmbedder(QObject* parent)
    : QObject(parent)
    , m_initialized(false)
{
    logToFile("PythonEmbedder constructor called");
}

PythonEmbedder::~PythonEmbedder() {
    logToFile("PythonEmbedder destructor called, calling shutdown");
    shutdown();
    logToFile("PythonEmbedder shutdown complete");
}

PythonEmbedder& PythonEmbedder::instance() {
    static PythonEmbedder instance;
    return instance;
}

bool PythonEmbedder::initialize() {
    logToFile("=== PythonEmbedder::initialize() START ===");
    qDebug() << "=== PythonEmbedder::initialize() START ===";
    
    if (m_initialized) {
        logToFile("Already initialized, returning true");
        qDebug() << "Already initialized, returning true";
        return true;
    }
    
    logToFile("Step 1: Setting Python path...");
    qDebug() << "Step 1: Setting Python path...";
    
    if (!setPythonPath()) {
        m_lastError = "Failed to set Python path";
        logToFile("ERROR: " + m_lastError);
        qCritical() << "ERROR:" << m_lastError;
        return false;
    }
    
    logToFile("Step 2: Setting PATH environment...");
    qDebug() << "Step 2: Setting PATH environment...";
    
    QString pythonDir = "C:/Users/32660/AppData/Local/Python/pythoncore-3.14-64";
    QString pythonDLLsDir = pythonDir + "/DLLs";
    QString pythonLibDir = pythonDir + "/Lib";
    QString pathEnv = QString::fromLocal8Bit(qgetenv("PATH"));
    if (!pathEnv.contains(pythonDir)) {
        qputenv("PATH", (pythonDLLsDir + ";" + pythonDir + ";" + pathEnv).toLocal8Bit());
        logToFile("Added to PATH: " + pythonDir + " and " + pythonDLLsDir);
        qDebug() << "Added to PATH:" << pythonDir << "and" << pythonDLLsDir;
    } else {
        logToFile("PATH already contains Python directory");
        qDebug() << "PATH already contains Python directory";
    }
    
    logToFile("Step 2b: Setting PYTHONHOME environment...");
    qDebug() << "Step 2b: Setting PYTHONHOME...";
    qputenv("PYTHONHOME", pythonDir.toLocal8Bit());
    logToFile("Set PYTHONHOME=" + pythonDir);
    
    logToFile("Step 2c: Setting PYTHONPATH environment...");
    qDebug() << "Step 2c: Setting PYTHONPATH...";
    qputenv("PYTHONPATH", pythonLibDir.toLocal8Bit());
    logToFile("Set PYTHONPATH=" + pythonLibDir);
    
    logToFile("Step 3: Calling Py_Initialize()...");
    qDebug() << "Step 3: Calling Py_Initialize()...";
    
    try {
        Py_Initialize();
        logToFile("Py_Initialize() completed without exception");
    } catch (const std::exception& e) {
        m_lastError = QString("Py_Initialize exception: %1").arg(e.what());
        logToFile("EXCEPTION: " + m_lastError);
        qCritical() << "EXCEPTION:" << m_lastError;
        return false;
    } catch (...) {
        m_lastError = "Py_Initialize: Unknown exception";
        logToFile("UNKNOWN EXCEPTION in Py_Initialize");
        qCritical() << "UNKNOWN EXCEPTION in Py_Initialize";
        return false;
    }
    
    logToFile("Step 4: Checking Py_IsInitialized()...");
    qDebug() << "Step 4: Checking Py_IsInitialized()...";
    
    if (!Py_IsInitialized()) {
        m_lastError = "Failed to initialize Python interpreter (Py_IsInitialized returned false)";
        logToFile("ERROR: " + m_lastError);
        qCritical() << "ERROR:" << m_lastError;
        return false;
    }
    
    logToFile("Step 5: Getting Python version...");
    qDebug() << "Step 5: Getting Python version...";
    
    const char* version = Py_GetVersion();
    logToFile(QString("Python version: %1").arg(version));
    qDebug() << "Python version:" << version;
    
    logToFile("Step 5b: Adding PythonEmail path to sys.path...");
    qDebug() << "Step 5b: Adding PythonEmail path to sys.path...";
    
    QString appPath = QCoreApplication::applicationDirPath();
    QStringList searchPaths = {
        appPath + "/PythonEmail",
        appPath + "/../PythonEmail",
        appPath + "/../../PythonEmail",
        appPath + "/../../../PythonEmail",
        appPath + "/../../../../PythonEmail",
        appPath + "/DataAssistant/PythonEmail",
        appPath + "/../../../DataAssistant/PythonEmail",
        appPath + "/../../../../DataAssistant/PythonEmail",
        "C:/CodeSpace/Objects/QTObj/PersonalDateAssisant/DataAssistant/PythonEmail"
    };
    
    for (const QString& path : searchPaths) {
        QDir dir(path);
        if (dir.exists()) {
            QString absPath = QDir::toNativeSeparators(QFileInfo(path).absoluteFilePath());
            PyObject* sysPath = PySys_GetObject("path");
            if (sysPath && PyList_Check(sysPath)) {
                PyObject* pathStr = PyUnicode_FromString(absPath.toUtf8().constData());
                if (pathStr) {
                    PyList_Insert(sysPath, 0, pathStr);
                    Py_DECREF(pathStr);
                    logToFile("Added PythonEmail to sys.path: " + absPath);
                    qDebug() << "Added PythonEmail to sys.path:" << absPath;
                }
            }
            break;
        }
    }
    
    logToFile("Step 6: Checking for PyEval_InitGIL...");
    qDebug() << "Step 6: Checking for PyEval_InitGIL...";
    
    m_initialized = true;
    logToFile("Set m_initialized = true");
    
    logToFile("Step 7: Emitting initialized signal...");
    qDebug() << "Step 7: Emitting initialized signal...";
    
    if (QCoreApplication::instance() && QCoreApplication::instance()->thread() == QThread::currentThread()) {
        emit initialized();
        logToFile("Signal emitted successfully");
    } else {
        QString msg = "Skipping emit - app:" + QString::number(QCoreApplication::instance() != nullptr) 
                    + " threadMatch:" + QString::number(QCoreApplication::instance() && QCoreApplication::instance()->thread() == QThread::currentThread());
        logToFile(msg);
        qDebug() << "Skipping emit:" << msg;
    }
    
    logToFile("=== PythonEmbedder::initialize() END ===");
    qDebug() << "=== PythonEmbedder::initialize() END ===";
    
    return true;
}

void PythonEmbedder::shutdown() {
    logToFile("PythonEmbedder::shutdown() called");
    qDebug() << "Shutting down Python interpreter...";
    
    if (m_initialized) {
        try {
            Py_Finalize();
            logToFile("Py_Finalize() completed");
        } catch (const std::exception& e) {
            logToFile(QString("Py_Finalize exception: %1").arg(e.what()));
        } catch (...) {
            logToFile("Py_Finalize: Unknown exception");
        }
        m_initialized = false;
        logToFile("Set m_initialized = false");
    } else {
        logToFile("Already shut down");
    }
}

bool PythonEmbedder::setPythonPath() {
    logToFile("setPythonPath() called");
    QString appPath = QCoreApplication::applicationDirPath();
    logToFile(QString("Application path: %1").arg(appPath));
    
    QString pythonEmailPath;
    QStringList searchPaths = {
        appPath + "/PythonEmail",
        appPath + "/../PythonEmail",
        appPath + "/../../PythonEmail",
        appPath + "/../../../PythonEmail",
        appPath + "/../../../../PythonEmail",
        appPath + "/DataAssistant/PythonEmail",
        appPath + "/../../../DataAssistant/PythonEmail",
        appPath + "/../../../../DataAssistant/PythonEmail",
        "C:/CodeSpace/Objects/QTObj/PersonalDateAssisant/DataAssistant/PythonEmail"
    };
    
    logToFile("Searching for PythonEmail directory...");
    for (const QString& path : searchPaths) {
        QDir dir(path);
        if (dir.exists()) {
            pythonEmailPath = QDir::toNativeSeparators(QFileInfo(path).absoluteFilePath());
            logToFile(QString("Found PythonEmail at: %1").arg(pythonEmailPath));
            break;
        }
    }
    
    if (pythonEmailPath.isEmpty()) {
        logToFile("ERROR: PythonEmail directory not found");
        qWarning() << "PythonEmail directory not found in any search path:";
        for (const QString& path : searchPaths) {
            qWarning() << "  -" << path;
            logToFile(QString("  Checked: %1").arg(path));
        }
        return false;
    }
    
    logToFile(QString("Adding Python path: %1").arg(pythonEmailPath));
    qDebug() << "Adding Python path:" << pythonEmailPath;
    
    if (m_initialized) {
        PyObject* sysPath = PySys_GetObject("path");
        if (sysPath && PyList_Check(sysPath)) {
            PyObject* pathStr = PyUnicode_FromString(pythonEmailPath.toUtf8().constData());
            if (pathStr) {
                PyList_Insert(sysPath, 0, pathStr);
                Py_DECREF(pathStr);
                logToFile("Python path added successfully");
            }
        }
    } else {
        logToFile("Python not initialized yet, path will be set by Py_InitializeEx");
    }
    
    return true;
}

QVariant PythonEmbedder::callPythonFunction(const QString& moduleName, const QString& functionName, const QVariantList& args) {
    logToFile(QString("callPythonFunction: %1.%2").arg(moduleName).arg(functionName));
    
    if (!m_initialized) {
        m_lastError = "Python interpreter not initialized";
        logToFile("ERROR: " + m_lastError);
        return QVariant();
    }
    
    PyGILState_STATE gstate = PyGILState_Ensure();
    logToFile("GIL acquired");
    
    PyObject* module = PyImport_ImportModule(moduleName.toUtf8().constData());
    if (!module) {
        m_lastError = QString("Failed to import module: %1").arg(moduleName);
        logToFile("ERROR: " + m_lastError);
        PyErr_Print();
        PyGILState_Release(gstate);
        return QVariant();
    }
    logToFile(QString("Module imported: %1").arg(moduleName));
    
    PyObject* func = PyObject_GetAttrString(module, functionName.toUtf8().constData());
    if (!func || !PyCallable_Check(func)) {
        m_lastError = QString("Failed to get function: %1.%2").arg(moduleName).arg(functionName);
        logToFile("ERROR: " + m_lastError);
        if (func) Py_DECREF(func);
        Py_DECREF(module);
        PyErr_Print();
        PyGILState_Release(gstate);
        return QVariant();
    }
    logToFile(QString("Function obtained: %1.%2").arg(moduleName).arg(functionName));
    
    PyObject* argsTuple = PyTuple_New(args.size());
    for (int i = 0; i < args.size(); ++i) {
        const QVariant& arg = args[i];
        PyObject* pyObj = nullptr;
        
        switch (arg.typeId()) {
            case QVariant::String:
                pyObj = PyUnicode_FromString(arg.toString().toUtf8().constData());
                break;
            case QVariant::Int:
            case QVariant::LongLong:
                pyObj = PyLong_FromLong(arg.toLongLong());
                break;
            case QVariant::UInt:
            case QVariant::ULongLong:
                pyObj = PyLong_FromUnsignedLongLong(arg.toULongLong());
                break;
            case QVariant::Double:
                pyObj = PyFloat_FromDouble(arg.toDouble());
                break;
            case QVariant::Bool:
                pyObj = arg.toBool() ? Py_True : Py_False;
                Py_INCREF(pyObj);
                break;
            case QVariant::List:
                {
                    QVariantList list = arg.toList();
                    pyObj = PyList_New(list.size());
                    for (int j = 0; j < list.size(); ++j) {
                        QVariant item = list[j];
                        PyObject* pyItem = nullptr;
                        if (item.canConvert<QString>()) {
                            pyItem = PyUnicode_FromString(item.toString().toUtf8().constData());
                        } else if (item.canConvert<int>()) {
                            pyItem = PyLong_FromLong(item.toInt());
                        } else if (item.canConvert<double>()) {
                            pyItem = PyFloat_FromDouble(item.toDouble());
                        } else {
                            pyItem = Py_None;
                            Py_INCREF(Py_None);
                        }
                        if (pyItem) {
                            PyList_SetItem(pyObj, j, pyItem);
                        }
                    }
                }
                break;
            case QVariant::Map:
                {
                    QVariantMap map = arg.toMap();
                    pyObj = PyDict_New();
                    for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
                        PyObject* key = PyUnicode_FromString(it.key().toUtf8().constData());
                        PyObject* value = nullptr;
                        if (it.value().canConvert<QString>()) {
                            value = PyUnicode_FromString(it.value().toString().toUtf8().constData());
                        } else if (it.value().canConvert<int>()) {
                            value = PyLong_FromLong(it.value().toInt());
                        } else {
                            value = Py_None;
                            Py_INCREF(Py_None);
                        }
                        if (key && value) {
                            PyDict_SetItem(pyObj, key, value);
                        }
                        if (key) Py_DECREF(key);
                        if (value && value != Py_None) Py_DECREF(value);
                    }
                }
                break;
            default:
                pyObj = Py_None;
                Py_INCREF(pyObj);
                break;
        }
        
        if (pyObj) {
            PyTuple_SetItem(argsTuple, i, pyObj);
        } else {
            Py_INCREF(Py_None);
            PyTuple_SetItem(argsTuple, i, Py_None);
        }
    }
    
    logToFile("Calling Python function...");
    PyObject* result = PyObject_CallObject(func, argsTuple);
    Py_DECREF(argsTuple);
    
    if (!result) {
        m_lastError = QString("Function call failed: %1.%2").arg(moduleName).arg(functionName);
        logToFile("ERROR: " + m_lastError);
        PyErr_Print();
        Py_DECREF(func);
        Py_DECREF(module);
        PyGILState_Release(gstate);
        return QVariant();
    }
    logToFile("Function call succeeded");
    
    QVariant qResult;
    
    if (PyBool_Check(result)) {
        qResult = result == Py_True;
    } else if (PyLong_Check(result)) {
        qResult = QVariant((qlonglong)PyLong_AsLongLong(result));
    } else if (PyFloat_Check(result)) {
        qResult = QVariant(PyFloat_AsDouble(result));
    } else if (PyUnicode_Check(result)) {
        qResult = PyUnicodeToQString(result);
    } else if (PyList_Check(result) || PyTuple_Check(result)) {
        QVariantList list;
        Py_ssize_t size = PyList_Check(result) ? PyList_Size(result) : PyTuple_Size(result);
        for (Py_ssize_t i = 0; i < size; ++i) {
            PyObject* item = PyList_Check(result) ? PyList_GetItem(result, i) : PyTuple_GetItem(result, i);
            if (PyUnicode_Check(item)) {
                list.append(PyUnicodeToQString(item));
            } else if (PyLong_Check(item)) {
                list.append((int)PyLong_AsLongLong(item));
            } else if (PyBool_Check(item)) {
                list.append(item == Py_True);
            } else if (PyDict_Check(item)) {
                QVariantMap subMap;
                PyObject *mapKey, *mapValue;
                Py_ssize_t mapPos = 0;
                while (PyDict_Next(item, &mapPos, &mapKey, &mapValue)) {
                    QString keyStr = PyUnicodeToQString(mapKey);
                    QVariant valueVar;
                    if (PyUnicode_Check(mapValue)) {
                        valueVar = PyUnicodeToQString(mapValue);
                    } else if (PyLong_Check(mapValue)) {
                        valueVar = (int)PyLong_AsLongLong(mapValue);
                    } else if (PyBool_Check(mapValue)) {
                        valueVar = mapValue == Py_True;
                    } else {
                        valueVar = QString();
                    }
                    subMap[keyStr] = valueVar;
                }
                list.append(subMap);
            } else if (PyList_Check(item) || PyTuple_Check(item)) {
                QVariantList subList;
                Py_ssize_t subSize = PyList_Check(item) ? PyList_Size(item) : PyTuple_Size(item);
                for (Py_ssize_t j = 0; j < subSize; ++j) {
                    PyObject* subItem = PyList_Check(item) ? PyList_GetItem(item, j) : PyTuple_GetItem(item, j);
                    if (PyUnicode_Check(subItem)) {
                        subList.append(PyUnicodeToQString(subItem));
                    } else if (PyLong_Check(subItem)) {
                        subList.append((int)PyLong_AsLongLong(subItem));
                    } else if (PyBool_Check(subItem)) {
                        subList.append(subItem == Py_True);
                    } else {
                        subList.append(QString());
                    }
                }
                list.append(subList);
            } else {
                list.append(QString());
            }
        }
        qResult = list;
    } else if (PyDict_Check(result)) {
        QVariantMap map;
        PyObject *key, *value;
        Py_ssize_t pos = 0;
        while (PyDict_Next(result, &pos, &key, &value)) {
            QString keyStr = PyUnicodeToQString(key);
            QVariant valueVar;
            if (PyUnicode_Check(value)) {
                valueVar = PyUnicodeToQString(value);
            } else if (PyLong_Check(value)) {
                valueVar = (int)PyLong_AsLongLong(value);
            } else if (PyBool_Check(value)) {
                valueVar = value == Py_True;
            }
            map[keyStr] = valueVar;
        }
        qResult = map;
    } else if (result == Py_None) {
        qResult = QVariant();
    }
    
    logToFile(QString("Result type: %1").arg(result->ob_type->tp_name));
    Py_DECREF(result);
    Py_DECREF(func);
    Py_DECREF(module);
    
    PyGILState_Release(gstate);
    logToFile("GIL released, function call complete");
    
    return qResult;
}

QVariant PythonEmbedder::evalPythonCode(const QString& code) {
    logToFile("evalPythonCode() called");
    
    if (!m_initialized) {
        m_lastError = "Python interpreter not initialized";
        logToFile("ERROR: " + m_lastError);
        return QVariant();
    }
    
    PyGILState_STATE gstate = PyGILState_Ensure();
    
    PyObject* mainModule = PyImport_AddModule("__main__");
    PyObject* globalDict = PyModule_GetDict(mainModule);
    PyObject* localDict = PyDict_New();
    
    logToFile("Executing Python code...");
    PyObject* result = PyRun_String(code.toUtf8().constData(), Py_file_input, globalDict, localDict);
    
    Py_DECREF(localDict);
    
    QVariant qResult;
    
    if (result) {
        if (PyBool_Check(result)) {
            qResult = result == Py_True;
        } else if (PyLong_Check(result)) {
            qResult = QVariant((qlonglong)PyLong_AsLongLong(result));
        } else if (PyFloat_Check(result)) {
            qResult = QVariant(PyFloat_AsDouble(result));
        } else if (PyUnicode_Check(result)) {
            qResult = PyUnicodeToQString(result);
        } else {
            qResult = QVariant(true);
        }
        Py_DECREF(result);
        logToFile("Code execution succeeded");
    } else {
        m_lastError = "Python code evaluation failed";
        logToFile("ERROR: " + m_lastError);
        PyErr_Print();
        qResult = QVariant(false);
    }
    
    PyGILState_Release(gstate);
    
    return qResult;
}