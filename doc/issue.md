[proc] 正在执行命令: F:\qt\Tools\CMake_64\bin\cmake.exe --build c:/CodeSpace/Objects/QTObj/PersonalDateAssisant/DataAssistant/build --config Debug --target all -j 12 --
[build] [  0%] Built target QtAwesome_autogen_timestamp_deps
[build] [  4%] Built target QtAwesome_autogen
[build] [ 34%] Built target QtAwesome
[build] [ 34%] Built target PersonalDateAssisant_autogen_timestamp_deps
[build] [ 39%] Automatic MOC and UIC for target PersonalDateAssisant
[build] [ 39%] Built target PersonalDateAssisant_autogen
[build] [ 43%] Building CXX object CMakeFiles/PersonalDateAssisant.dir/PersonalDateAssisant_autogen/mocs_compilation.cpp.obj
[build] [ 47%] Building CXX object CMakeFiles/PersonalDateAssisant.dir/main.cpp.obj
[build] [ 52%] Building CXX object CMakeFiles/PersonalDateAssisant.dir/Widget.cpp.obj
[build] [ 56%] Building CXX object CMakeFiles/PersonalDateAssisant.dir/DatabaseManager.cpp.obj
[build] [ 60%] Building CXX object CMakeFiles/PersonalDateAssisant.dir/ScheduleDialog.cpp.obj
[build] [ 65%] Building CXX object CMakeFiles/PersonalDateAssisant.dir/ReminderManager.cpp.obj
[build] [ 69%] Building CXX object CMakeFiles/PersonalDateAssisant.dir/SettingsDialog.cpp.obj
[build] [ 73%] Building CXX object CMakeFiles/PersonalDateAssisant.dir/ScheduleDetailDialog.cpp.obj
[build] [ 78%] Building CXX object CMakeFiles/PersonalDateAssisant.dir/ScheduleListDialog.cpp.obj
[build] [ 82%] Building CXX object CMakeFiles/PersonalDateAssisant.dir/CalendarDelegate.cpp.obj
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp: In constructor 'ScheduleListDialog::ScheduleListDialog(QWidget*)':
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:14:7: error: class 'ScheduleListDialog' does not have any field named 'm_batchDelete'
[build]    14 |     , m_batchDelete(nullptr)
[build]       |       ^~~~~~~~~~~~~
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:15:7: error: class 'ScheduleListDialog' does not have any field named 'm_selectAll'
[build]    15 |     , m_selectAll(nullptr)
[build]       |       ^~~~~~~~~~~
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:16:7: error: class 'ScheduleListDialog' does not have any field named 'm_deselectAll'
[build]    16 |     , m_deselectAll(nullptr)
[build]       |       ^~~~~~~~~~~~~
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:27:41: error: 'WindowContextHelpHint' is not a member of 'Qt'; did you mean 'WindowContextHelpButtonHint'?
[build]    27 |     setWindowFlags(windowFlags() & ~Qt::WindowContextHelpHint);
[build]       |                                         ^~~~~~~~~~~~~~~~~~~~~
[build]       |                                         WindowContextHelpButtonHint
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp: In member function 'void ScheduleListDialog::setupUI()':
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:55:5: error: 'm_selectAll' was not declared in this scope; did you mean 'onSelectAll'?
[build]    55 |     m_selectAll = new QPush("鈽� 鍏ㄩ€�");
[build]       |     ^~~~~~~~~~~
[build]       |     onSelectAll
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:55:23: error: expected type-specifier before 'QPush'
[build]    55 |     m_selectAll = new QPush("鈽� 鍏ㄩ€�");
[build]       |                       ^~~~~
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:74:27: error: 'QPush' has not been declared
[build]    74 |     connect(m_selectAll, &QPush::clicked, this, &ScheduleListDialog::onSelectAll);
[build]       |                           ^~~~~
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:78:5: error: 'm_deselectAll' was not declared in this scope; did you mean 'onDeselectAll'?
[build]    78 |     m_deselectAll = new QPush("鈽� 鍙栨秷鍏ㄩ€�");
[build]       |     ^~~~~~~~~~~~~
[build]       |     onDeselectAll
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:78:25: error: expected type-specifier before 'QPush'
[build]    78 |     m_deselectAll = new QPush("鈽� 鍙栨秷鍏ㄩ€�");
[build]       |                         ^~~~~
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:97:29: error: 'QPush' has not been declared
[build]    97 |     connect(m_deselectAll, &QPush::clicked, this, &ScheduleListDialog::onDeselectAll);
[build]       |                             ^~~~~
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:108:5: error: 'QPush' was not declared in this scope
[build]   108 |     QPush* deleteAll = new QPush("馃棏 娓呯┖鎵€鏈�");
[build]       |     ^~~~~
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:108:12: error: 'deleteAll' was not declared in this scope; did you mean 'qDeleteAll'?
[build]   108 |     QPush* deleteAll = new QPush("馃棏 娓呯┖鎵€鏈�");
[build]       |            ^~~~~~~~~
[build]       |            qDeleteAll
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:108:28: error: expected type-specifier before 'QPush'
[build]   108 |     QPush* deleteAll = new QPush("馃棏 娓呯┖鎵€鏈�");
[build]       |                            ^~~~~
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:127:25: error: 'QPush' is not a class, namespace, or enumeration
[build]   127 |     connect(deleteAll, &QPush::clicked, this, &ScheduleListDialog::onDeleteAll);
[build]       |                         ^~~~~
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:131:5: error: 'm_batchDelete' was not declared in this scope; did you mean 'onBatchDelete'?
[build]   131 |     m_batchDelete = new QPush("馃棏 鎵归噺鍒犻櫎");
[build]       |     ^~~~~~~~~~~~~
[build]       |     onBatchDelete
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:131:25: error: expected type-specifier before 'QPush'
[build]   131 |     m_batchDelete = new QPush("馃棏 鎵归噺鍒犻櫎");
[build]       |                         ^~~~~
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:155:29: error: 'QPush' is not a class, namespace, or enumeration
[build]   155 |     connect(m_batchDelete, &QPush::clicked, this, &ScheduleListDialog::onBatchDelete);
[build]       |                             ^~~~~
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp: In member function 'QWidget* ScheduleListDialog::createScheduleItem(const Schedule&)':
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:483:5: error: 'QPush' was not declared in this scope
[build]   483 |     QPush* minimize = new QPush();
[build]       |     ^~~~~
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:483:12: error: 'minimize' was not declared in this scope; did you mean 'minimumSize'?
[build]   483 |     QPush* minimize = new QPush();
[build]       |            ^~~~~~~~
[build]       |            minimumSize
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:483:27: error: expected type-specifier before 'QPush'
[build]   483 |     QPush* minimize = new QPush();
[build]       |                           ^~~~~
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:511:24: error: 'QPush' is not a class, namespace, or enumeration
[build]   511 |     connect(minimize, &QPush::clicked, this, [this, scheduleId, itemWidget, animation, minimize]() {
[build]       |                        ^~~~~
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp: In lambda function:
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:519:9: error: 'minimize' is not captured
[build]   519 |         minimize->setText(minimized ? "馃搵" : "馃摑");
[build]       |         ^~~~~~~~
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:511:96: note: the lambda has no capture-default
[build]   511 |     connect(minimize, &QPush::clicked, this, [this, scheduleId, itemWidget, animation, minimize]() {
[build]       |                                                                                                ^
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:483:12: note: '<typeprefixerror>minimize' declared here
[build]   483 |     QPush* minimize = new QPush();
[build]       |            ^~~~~~~~
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp: In member function 'QWidget* ScheduleListDialog::createScheduleItem(const Schedule&)':
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:536:12: error: 'edit' was not declared in this scope; did you mean 'exit'?
[build]   536 |     QPush* edit = new QPush("缂栬緫");
[build]       |            ^~~~
[build]       |            exit
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:536:23: error: expected type-specifier before 'QPush'
[build]   536 |     QPush* edit = new QPush("缂栬緫");
[build]       |                       ^~~~~
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:557:20: error: 'QPush' is not a class, namespace, or enumeration
[build]   557 |     connect(edit, &QPush::clicked, this, [this, schedule]() {
[build]       |                    ^~~~~
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:563:19: error: expected primary-expression before '=' token
[build]   563 |     QPush* delete = new QPush("鍒犻櫎");
[build]       |                   ^
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:563:25: error: expected type-specifier before 'QPush'
[build]   563 |     QPush* delete = new QPush("鍒犻櫎");
[build]       |                         ^~~~~
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:564:11: error: expected primary-expression before '->' token
[build]   564 |     delete->setCursor(Qt::PointingHandCursor);
[build]       |           ^~
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:565:11: error: expected primary-expression before '->' token
[build]   565 |     delete->setFixedHeight(28);
[build]       |           ^~
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:566:11: error: expected primary-expression before '->' token
[build]   566 |     delete->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
[build]       |           ^~
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:567:11: error: expected primary-expression before '->' token
[build]   567 |     delete->setStyleSheet(R"(
[build]       |           ^~
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:584:19: error: expected primary-expression before ',' token
[build]   584 |     connect(delete, &QPush::clicked, this, [this, schedule]() {
[build]       |                   ^
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:584:22: error: 'QPush' is not a class, namespace, or enumeration
[build]   584 |     connect(delete, &QPush::clicked, this, [this, schedule]() {
[build]       |                      ^~~~~
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:587:34: error: expected primary-expression before ')' token
[build]   587 |     GroupLayout->addWidget(delete);
[build]       |                                  ^
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp: In member function 'void ScheduleListDialog::toggleScheduleSelection(int, bool)':
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:686:5: error: 'updateBatchDelete' was not declared in this scope; did you mean 'onBatchDelete'?
[build]   686 |     updateBatchDelete();
[build]       |     ^~~~~~~~~~~~~~~~~
[build]       |     onBatchDelete
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp: At global scope:
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:689:6: error: no declaration matches 'void ScheduleListDialog::updateBatchDelete()'
[build]   689 | void ScheduleListDialog::updateBatchDelete()
[build]       |      ^~~~~~~~~~~~~~~~~~
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:689:6: note: no functions named 'void ScheduleListDialog::updateBatchDelete()'
[build] In file included from C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:1:
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.h:18:7: note: 'class ScheduleListDialog' defined here
[build]    18 | class ScheduleListDialog : public QDialog
[build]       |       ^~~~~~~~~~~~~~~~~~
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp: In member function 'void ScheduleListDialog::onSelectAll()':
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:709:5: error: 'updateBatchDelete' was not declared in this scope; did you mean 'onBatchDelete'?
[build]   709 |     updateBatchDelete();
[build]       |     ^~~~~~~~~~~~~~~~~
[build]       |     onBatchDelete
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp: In member function 'void ScheduleListDialog::onDeselectAll()':
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:716:5: error: 'updateBatchDelete' was not declared in this scope; did you mean 'onBatchDelete'?
[build]   716 |     updateBatchDelete();
[build]       |     ^~~~~~~~~~~~~~~~~
[build]       |     onBatchDelete
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp: In member function 'void ScheduleListDialog::onBatchDelete()':
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:758:18: error: 'Standard' is not a member of 'QMessageBox'
[build]   758 |     QMessageBox::Standard reply = QMessageBox::question(
[build]       |                  ^~~~~~~~
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:766:9: error: 'reply' was not declared in this scope
[build]   766 |     if (reply != QMessageBox::Yes) {
[build]       |         ^~~~~
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp: In lambda function:
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:795:9: error: 'updateBatchDelete' was not declared in this scope; did you mean 'onBatchDelete'?
[build]   795 |         updateBatchDelete();
[build]       |         ^~~~~~~~~~~~~~~~~
[build]       |         onBatchDelete
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp: In member function 'void ScheduleListDialog::onDeleteAll()':
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:837:18: error: 'Standard' is not a member of 'QMessageBox'
[build]   837 |     QMessageBox::Standard reply = QMessageBox::question(
[build]       |                  ^~~~~~~~
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:845:9: error: 'reply' was not declared in this scope
[build]   845 |     if (reply != QMessageBox::Yes) {
[build]       |         ^~~~~
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp: In lambda function:
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:878:9: error: 'updateBatchDelete' was not declared in this scope; did you mean 'onBatchDelete'?
[build]   878 |         updateBatchDelete();
[build]       |         ^~~~~~~~~~~~~~~~~
[build]       |         onBatchDelete
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp: In member function 'void ScheduleListDialog::onDeleteSchedule(int)':
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:940:18: error: 'Standard' is not a member of 'QMessageBox'
[build]   940 |     QMessageBox::Standard reply = QMessageBox::question(
[build]       |                  ^~~~~~~~
[build] C:\CodeSpace\Objects\QTObj\PersonalDateAssisant\DataAssistant\ScheduleListDialog.cpp:947:9: error: 'reply' was not declared in this scope
[build]   947 |     if (reply == QMessageBox::Yes) {
[build]       |         ^~~~~
[build] mingw32-make.exe[2]: *** [CMakeFiles\PersonalDateAssisant.dir\build.make:218: CMakeFiles/PersonalDateAssisant.dir/ScheduleListDialog.cpp.obj] Error 1
[build] mingw32-make.exe[2]: *** Waiting for unfinished jobs....
[build] mingw32-make.exe[1]: *** [CMakeFiles\Makefile2:105: CMakeFiles/PersonalDateAssisant.dir/all] Error 2
[build] mingw32-make.exe: *** [Makefile:155: all] Error 2
[proc] 命令“F:\qt\Tools\CMake_64\bin\cmake.exe --build c:/CodeSpace/Objects/QTObj/PersonalDateAssisant/DataAssistant/build --config Debug --target all -j 12 --”已退出，代码为 2
[driver] 生成完毕: 00:00:06.252
[build] 生成已完成，退出代码为 2