#include "selfdrive/ui/qt/offroad/settings.h"

#include <cassert>
#include <string>

#include <QDebug>

#ifndef QCOM
#include "selfdrive/ui/qt/offroad/networking.h"
#endif

#ifdef ENABLE_MAPS
#include "selfdrive/ui/qt/maps/map_settings.h"
#endif

#include "selfdrive/common/params.h"
#include "selfdrive/common/util.h"
#include "selfdrive/hardware/hw.h"
#include "selfdrive/ui/qt/widgets/controls.h"
#include "selfdrive/ui/qt/widgets/input.h"
#include "selfdrive/ui/qt/widgets/scrollview.h"
#include "selfdrive/ui/qt/widgets/ssh_keys.h"
#include "selfdrive/ui/qt/widgets/toggle.h"
#include "selfdrive/ui/ui.h"
#include "selfdrive/ui/qt/util.h"
#include "selfdrive/ui/qt/qt_window.h"

#include <QComboBox>
#include <QAbstractItemView>
#include <QScroller>
#include <QListView>

TogglesPanel::TogglesPanel(QWidget *parent) : QWidget(parent) {
  QVBoxLayout *main_layout = new QVBoxLayout(this);

  QList<ParamControl*> toggles;

  toggles.append(new ParamControl("OpenpilotEnabledToggle",
                                  "오픈파일럿 사용",
                                  "어댑티브 크루즈 컨트롤 및 차선 유지 지원을 위해 오픈파일럿 시스템을 사용하십시오. 이 기능을 사용하려면 항상 주의를 기울여야 합니다. 이 설정을 변경하는 것은 자동차의 전원이 꺼졌을 때 적용됩니다.",
                                  "../assets/offroad/icon_openpilot.png",
                                  this));
  toggles.append(new ParamControl("IsLdwEnabled",
                                  "차선이탈 경보 사용",
                                  "50km/h이상의 속도로 주행하는 동안 방향 지시등이 활성화되지 않은 상태에서 차량이 감지된 차선 위를 넘어갈 경우 원래 차선으로 다시 방향을 전환하도록 경고를 보냅니다.",
                                  "../assets/offroad/icon_warning.png",
                                  this));
  toggles.append(new ParamControl("IsRHD",
                                  "우핸들 운전방식 사용",
                                  "오픈파일럿이 좌측 교통 규칙을 준수하도록 허용하고 우측 운전석에서 운전자 모니터링을 수행하십시오.",
                                  "../assets/offroad/icon_openpilot_mirrored.png",
                                  this));
  toggles.append(new ParamControl("IsMetric",
                                  "미터법 사용",
                                  "mi/h 대신 km/h 단위로 속도를 표시합니다.",
                                  "../assets/offroad/icon_metric.png",
                                  this));
  toggles.append(new ParamControl("CommunityFeaturesToggle",
                                  "커뮤니티 기능 사용",
                                  "comma.ai에서 유지 또는 지원하지 않고 표준 안전 모델에 부합하는 것으로 확인되지 않은 오픈 소스 커뮤니티의 기능을 사용하십시오. 이러한 기능에는 커뮤니티 지원 자동차와 커뮤니티 지원 하드웨어가 포함됩니다. 이러한 기능을 사용할 때는 각별히 주의해야 합니다.",
                                  "../assets/offroad/icon_shell.png",
                                  this));

  toggles.append(new ParamControl("UploadRaw",
                                 "원본로그 업로드",
                                 "Wi-Fi에 연결하면 기본적으로 전체 로그 및 전체 해상도 비디오를 업로드합니다. 활성화되지 않은 경우 my.comma.ai/useradmin에서 개별 로그를 업로드 용으로 표시 할 수 있습니다.",
                                  "../assets/offroad/icon_network.png",
                                  this));

  ParamControl *record_toggle = new ParamControl("RecordFront",
                                                 "운전자 영상 녹화 및 업로드",
                                                "운전자 모니터링 카메라에서 데이터를 업로드하고 운전자 모니터링 알고리즘을 개선하십시오.",
                                                "../assets/offroad/icon_monitoring.png",
                                                this);
  toggles.append(record_toggle);
  toggles.append(new ParamControl("EndToEndToggle",
                                   "\U0001f96c 차선이 없을 때 사용 버전 (알파) \U0001f96c",
                                   "이 모드에서 openpilot은 차선을 무시하고 사람이 생각하는대로 운전합니다.",
                                   "../assets/offroad/icon_road.png",
                                   this));

#ifdef ENABLE_MAPS
  toggles.append(new ParamControl("NavSettingTime24h",
                                  "Show ETA in 24h format",
                                  "Use 24h format instead of am/pm",
                                  "../assets/offroad/icon_metric.png",
                                  this));
#endif

  bool record_lock = Params().getBool("RecordFrontLock");
  record_toggle->setEnabled(!record_lock);

  for(ParamControl *toggle : toggles) {
    if(main_layout->count() != 0) {
      main_layout->addWidget(horizontal_line());
    }
    main_layout->addWidget(toggle);
  }
}

DevicePanel::DevicePanel(QWidget* parent) : QWidget(parent) {
  QVBoxLayout *main_layout = new QVBoxLayout(this);
  Params params = Params();
  main_layout->addWidget(new LabelControl("동글 ID", getDongleId().value_or("N/A")));
  main_layout->addWidget(horizontal_line());

  QString serial = QString::fromStdString(params.get("HardwareSerial", false));
  main_layout->addWidget(new LabelControl("시리얼", serial));

  QHBoxLayout *reset_layout = new QHBoxLayout();
  reset_layout->setSpacing(30);

  // reset calibration button
  QPushButton *reset_calib_btn = new QPushButton("캘리브레이션과 라이브파라미터 리셋");
  reset_calib_btn->setStyleSheet("height: 120px;border-radius: 15px;background-color: #393939;");
  reset_layout->addWidget(reset_calib_btn);
  QObject::connect(reset_calib_btn, &QPushButton::released, [=]() {
    if (ConfirmationDialog::confirm("캘리브레이션과 라이브파라미터를 초기화 하시겠습니까?", this)) {
      Params().remove("CalibrationParams");
      Params().remove("LiveParameters");
      QTimer::singleShot(1000, []() {
        Hardware::reboot();
      });
    }
  });

  main_layout->addWidget(horizontal_line());
  main_layout->addLayout(reset_layout);

  // offroad-only buttons

  auto dcamBtn = new ButtonControl("운전자 영상", "미리보기",
                                        "운전자 영상이 제대로 작동하는지 확인을 합니다. 차량을 끄고 사용하도록 하십시오.");
  connect(dcamBtn, &ButtonControl::clicked, [=]() { emit showDriverView(); });

  QString resetCalibDesc = "오픈파일럿은 좌우로 4° 위아래로 5° 를 보정합니다. 그 이상의 경우 보정이 필요합니다.";
  auto resetCalibBtn = new ButtonControl("캘리브레이션 초기화", "초기화", resetCalibDesc);
  connect(resetCalibBtn, &ButtonControl::clicked, [=]() {
    if (ConfirmationDialog::confirm("캘리브레이션을 초기화 하시겠습니까?", this)) {
      Params().remove("CalibrationParams");
    }
  });
  connect(resetCalibBtn, &ButtonControl::showDescription, [=]() {
    QString desc = resetCalibDesc;
    std::string calib_bytes = Params().get("CalibrationParams");
    if (!calib_bytes.empty()) {
      try {
        AlignedBuffer aligned_buf;
        capnp::FlatArrayMessageReader cmsg(aligned_buf.align(calib_bytes.data(), calib_bytes.size()));
        auto calib = cmsg.getRoot<cereal::Event>().getLiveCalibration();
        if (calib.getCalStatus() != 0) {
          double pitch = calib.getRpyCalib()[1] * (180 / M_PI);
          double yaw = calib.getRpyCalib()[2] * (180 / M_PI);
          desc += QString("\n장치가 %1° %2 그리고 %3° %4 위치해 있습니다.")
                                .arg(QString::number(std::abs(pitch), 'g', 1), pitch > 0 ? "위로" : "아래로",
                                     QString::number(std::abs(yaw), 'g', 1), yaw > 0 ? "오른쪽으로" : "왼쪽으로");
        }
      } catch (kj::Exception) {
        qInfo() << "캘리브레이션 파라미터 유효하지 않음";
      }
    }
    resetCalibBtn->setDescription(desc);
  });

  ButtonControl *retrainingBtn = nullptr;
  if (!params.getBool("Passive")) {
    retrainingBtn = new ButtonControl("트레이닝 가이드", "가이드 보기", "오픈파일럿의 제한적 상황과 규정을 확인");
    connect(retrainingBtn, &ButtonControl::clicked, [=]() {
      if (ConfirmationDialog::confirm("트레이닝 가이드를 확인하시겠습니까?", this)) {
        Params().remove("CompletedTrainingVersion");
        emit reviewTrainingGuide();
      }
    });
  }

  ButtonControl *regulatoryBtn = nullptr;
  if (Hardware::TICI()) {
    regulatoryBtn = new ButtonControl("Regulatory", "VIEW", "");
    connect(regulatoryBtn, &ButtonControl::clicked, [=]() {
      const std::string txt = util::read_file(ASSET_PATH.toStdString() + "/offroad/fcc.html");
      RichTextDialog::alert(QString::fromStdString(txt), this);
    });
  }

  for (auto btn : {dcamBtn, resetCalibBtn, retrainingBtn, regulatoryBtn}) {
    if (btn) {
      main_layout->addWidget(horizontal_line());
      connect(parent, SIGNAL(offroadTransition(bool)), btn, SLOT(setEnabled(bool)));
      main_layout->addWidget(btn);
    }
  }

  // power buttons
  QHBoxLayout *power_layout = new QHBoxLayout();
  power_layout->setSpacing(30);

  QPushButton *reboot_btn = new QPushButton("재부팅");
  reboot_btn->setObjectName("reboot_btn");
  power_layout->addWidget(reboot_btn);
  QObject::connect(reboot_btn, &QPushButton::clicked, [=]() {
    if (ConfirmationDialog::confirm("장치를 재부팅 하시겠습니까?", this)) {
      Hardware::reboot();
    }
  });

  QPushButton *poweroff_btn = new QPushButton("전원 종료");
  poweroff_btn->setObjectName("poweroff_btn");
  power_layout->addWidget(poweroff_btn);
  QObject::connect(poweroff_btn, &QPushButton::clicked, [=]() {
    if (ConfirmationDialog::confirm("전원을 종료하시겠습니까?", this)) {
      Hardware::poweroff();
    }
  });

  setStyleSheet(R"(
    QPushButton {
      height: 120px;
      border-radius: 15px;
    }
    #reboot_btn { background-color: #393939; }
    #reboot_btn:pressed { background-color: #4a4a4a; }
    #poweroff_btn { background-color: #E22C2C; }
    #poweroff_btn:pressed { background-color: #FF2424; }
  )");
  main_layout->addLayout(power_layout);
}

SoftwarePanel::SoftwarePanel(QWidget* parent) : QWidget(parent) {
  gitBranchLbl = new LabelControl("Git Branch");
  gitCommitLbl = new LabelControl("Git Commit");
  osVersionLbl = new LabelControl("OS Version");
  versionLbl = new LabelControl("Version", "", QString::fromStdString(params.get("ReleaseNotes")).trimmed());
  lastUpdateLbl = new LabelControl("Last Update Check", "", "The last time openpilot successfully checked for an update. The updater only runs while the car is off.");
  updateBtn = new ButtonControl("Check for Update", "");
  connect(updateBtn, &ButtonControl::clicked, [=]() {
    if (params.getBool("IsOffroad")) {
      const QString paramsPath = QString::fromStdString(params.getParamsPath());
      fs_watch->addPath(paramsPath + "/d/LastUpdateTime");
      fs_watch->addPath(paramsPath + "/d/UpdateFailedCount");
      updateBtn->setText("CHECKING");
      updateBtn->setEnabled(false);
    }
    std::system("pkill -1 -f selfdrive.updated");
  });

  QVBoxLayout *main_layout = new QVBoxLayout(this);
  QWidget *widgets[] = {versionLbl, lastUpdateLbl, updateBtn, gitBranchLbl, gitCommitLbl, osVersionLbl};
  for (int i = 0; i < std::size(widgets); ++i) {
    main_layout->addWidget(widgets[i]);
    main_layout->addWidget(horizontal_line());
  }

  auto uninstallBtn = new ButtonControl("Uninstall " + getBrand(), "UNINSTALL");
  connect(uninstallBtn, &ButtonControl::clicked, [=]() {
    if (ConfirmationDialog::confirm("삭제를 진행할까요?", this)) {
      Params().putBool("DoUninstall", true);
    }
  });
  connect(parent, SIGNAL(offroadTransition(bool)), uninstallBtn, SLOT(setEnabled(bool)));
  main_layout->addWidget(uninstallBtn);

  fs_watch = new QFileSystemWatcher(this);
  QObject::connect(fs_watch, &QFileSystemWatcher::fileChanged, [=](const QString path) {
    int update_failed_count = params.get<int>("UpdateFailedCount").value_or(0);
    if (path.contains("UpdateFailedCount") && update_failed_count > 0) {
      lastUpdateLbl->setText("failed to fetch update");
      updateBtn->setText("CHECK");
      updateBtn->setEnabled(true);
    } else if (path.contains("LastUpdateTime")) {
      updateLabels();
    }
  });
}

void SoftwarePanel::showEvent(QShowEvent *event) {
  updateLabels();
}

void SoftwarePanel::updateLabels() {
  QString lastUpdate = "";
  auto tm = params.get("LastUpdateTime");
  if (!tm.empty()) {
    lastUpdate = timeAgo(QDateTime::fromString(QString::fromStdString(tm + "Z"), Qt::ISODate));
  }

  versionLbl->setText(getBrandVersion());
  lastUpdateLbl->setText(lastUpdate);
  updateBtn->setText("CHECK");
  updateBtn->setEnabled(true);
  gitBranchLbl->setText(QString::fromStdString(params.get("GitBranch")));
  gitCommitLbl->setText(QString::fromStdString(params.get("GitCommit")).left(10));
  osVersionLbl->setText(QString::fromStdString(Hardware::get_os_version()).trimmed());
}

QWidget * network_panel(QWidget * parent) {
#ifdef QCOM
  QWidget *w = new QWidget(parent);
  QVBoxLayout *layout = new QVBoxLayout(w);
  layout->setSpacing(30);

  // wifi + tethering buttons
  auto wifiBtn = new ButtonControl("네트워크 설정열기", "열기");
  QObject::connect(wifiBtn, &ButtonControl::clicked, [=]() { HardwareEon::launch_wifi(); });
  layout->addWidget(wifiBtn);
  layout->addWidget(horizontal_line());

  auto tetheringBtn = new ButtonControl("테더링 설정열기", "열기");
  QObject::connect(tetheringBtn, &ButtonControl::clicked, [=]() { HardwareEon::launch_tethering(); });
  layout->addWidget(tetheringBtn);
  layout->addWidget(horizontal_line());

  // SSH key management
  layout->addWidget(new SshToggle());
  layout->addWidget(horizontal_line());
  layout->addWidget(new SshControl());

  layout->addStretch(1);
#else
  Networking *w = new Networking(parent);
#endif
  return w;
}

QStringList get_list(const char* path)
{
  QStringList stringList;
  QFile textFile(path);
  if(textFile.open(QIODevice::ReadOnly))
  {
      QTextStream textStream(&textFile);
      while (true)
      {
        QString line = textStream.readLine();
        if (line.isNull())
            break;
        else
            stringList.append(line);
      }
  }

  return stringList;
}

QWidget * community_panel() {
  QVBoxLayout *toggles_list = new QVBoxLayout();
  //toggles_list->setMargin(50);

  QComboBox* supported_cars = new QComboBox();
  supported_cars->setStyleSheet(R"(
  QComboBox {
    background-color: #393939;
    border-radius: 15px;
    padding-left: 40px
    height: 140px;
    }
)");
  QListView* list = new QListView(supported_cars);
  list->setStyleSheet("QListView {padding: 40px; background-color: #393939; border-radius: 15px; height: 140px;} QListView::item{height: 100px}");
  supported_cars->setView(list);

  QScroller::grabGesture(supported_cars->view()->viewport(),QScroller::LeftMouseButtonGesture);
  supported_cars->view()->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

  supported_cars->setMinimumSize(200, 120);

  supported_cars->addItem("[ Select your car ]");
  supported_cars->addItems(get_list("/data/params/d/SupportedCars"));

  supported_cars->setCurrentText(QString(Params().get("SelectedCar").c_str()));

  QObject::connect(supported_cars, QOverload<int>::of(&QComboBox::currentIndexChanged),
    [=](int index){

        if(supported_cars->currentIndex() == 0)
            Params().remove("SelectedCar");
        else
            Params().put("SelectedCar", supported_cars->currentText().toStdString());

    });

  toggles_list->addWidget(supported_cars);


  toggles_list->addWidget(horizontal_line());
  toggles_list->addWidget(new ParamControl("UseClusterSpeed",
                                            "계기반 속도 사용",
                                            "휠 속도 대신에 계기반 속도 사용",
                                            "../assets/offroad/icon_road.png"
                                              ));

  toggles_list->addWidget(horizontal_line());
  toggles_list->addWidget(new ParamControl("LongControlEnabled",
                                            "롱컨트롤 사용",
                                            "경고 : 오픈파일럿이 속도를 조절합니다. 주의 하시길 바랍니다.",
                                            "../assets/offroad/icon_road.png"
                                              ));
  toggles_list->addWidget(horizontal_line());
  toggles_list->addWidget(new ParamControl("MadModeEnabled",
                                            "매드모드 사용",
                                            "HKG 매드모드 사용. 가감속을 사용 하지 않아도 핸들 조향을 사용합니다.",
                                            "../assets/offroad/icon_openpilot.png"
                                              ));
  toggles_list->addWidget(horizontal_line());
  toggles_list->addWidget(new ParamControl("IsLdwsCar",
                                            "LDWS",
                                            "LDWS 옵션인 경우 사용",
                                            "../assets/offroad/icon_openpilot.png"
                                              ));
  toggles_list->addWidget(horizontal_line());
  toggles_list->addWidget(new ParamControl("LaneChangeEnabled",
                                            "차선 변경 사용",
                                            "주변의 안전을 확인하고, 방향 지시등을 활성화하고, 원하는 차선을 향해 스티어링 휠을 부드럽게 밀어서 오픈 파일럿으로 보조 차선 변경을 수행하십시오. openpilot은 차선 변경이 안전한지 확인할 수 없습니다. 이 기능을 사용하려면 주변을 지속적으로 관찰해야합니다.",
                                            "../assets/offroad/icon_road.png"
                                              ));
  toggles_list->addWidget(horizontal_line());
  toggles_list->addWidget(new ParamControl("AutoLaneChangeEnabled",
                                            "자동 차선변경 사용-(핸들조작없이 차선변경)",
                                            "경고 : 베타이기 때문에 조심히 사용하세요!!",
                                            "../assets/offroad/icon_road.png"
                                              ));  
  toggles_list->addWidget(horizontal_line());
  toggles_list->addWidget(new ParamControl("SccSmootherState",
                                            "가감속 스무서 사용",
                                            "N Smoother. 순정 ASCC 기능을 이용하여 가속과 감속을 부드럽게 할 수 있도록 도와 줍니다. 사용법을 정확히 인지하고 사용하십시오.",
                                            "../assets/offroad/icon_road.png"
                                            ));
  toggles_list->addWidget(horizontal_line());
  toggles_list->addWidget(new ParamControl("SccSmootherSlowOnCurves",
                                            "커브 감속 사용",
                                            "",
                                            "../assets/offroad/icon_road.png"
                                            ));
  toggles_list->addWidget(horizontal_line());
  toggles_list->addWidget(new ParamControl("SccSmootherSyncGasPressed",
                                            "크루즈 속도의 동기화",
                                            "크루즈 속도를 설정 후 엑셀로 인해 설정 속도보다 가속 속도가 높아지면 그 속도에 크루즈 설정 속도가 동기화 됩니다.",
                                            "../assets/offroad/icon_road.png"
                                            ));
  toggles_list->addWidget(horizontal_line());
  toggles_list->addWidget(new ParamControl("StockNaviDecelEnabled",
                                            "Stock Navi based deceleration",
                                            "Use the stock navi based deceleration for longcontrol",
                                            "../assets/offroad/icon_road.png"
                                            ));
  toggles_list->addWidget(horizontal_line());
  toggles_list->addWidget(new ParamControl("ShowDebugUI",
                                            "디버그 내용 보기",
                                            "가감속 등 디버그 내용을 화면에 띄웁니다.",
                                            "../assets/offroad/icon_shell.png"
                                            ));

  toggles_list->addWidget(horizontal_line());
  toggles_list->addWidget(new ParamControl("CustomLeadMark",
                                            "선행차 사용자 정의 마크 사용",
                                            "",
                                            "../assets/offroad/icon_road.png"
                                            ));

  QWidget *widget = new QWidget;
  widget->setLayout(toggles_list);
  return widget;
}

void SettingsWindow::showEvent(QShowEvent *event) {
  panel_widget->setCurrentIndex(0);
  nav_btns->buttons()[0]->setChecked(true);
}

SettingsWindow::SettingsWindow(QWidget *parent) : QFrame(parent) {

  // setup two main layouts
  sidebar_widget = new QWidget;
  QVBoxLayout *sidebar_layout = new QVBoxLayout(sidebar_widget);
  sidebar_layout->setMargin(0);
  panel_widget = new QStackedWidget();
  panel_widget->setStyleSheet(R"(
    border-radius: 30px;
    background-color: #292929;
  )");

  // close button
  QPushButton *close_btn = new QPushButton("×");
  close_btn->setStyleSheet(R"(
    QPushButton {
      font-size: 140px;
      padding-bottom: 20px;
      font-weight: bold;
      border 1px grey solid;
      border-radius: 100px;
      background-color: #292929;
      font-weight: 400;
    }
    QPushButton:pressed {
      background-color: #3B3B3B;
    }
  )");
  close_btn->setFixedSize(200, 200);
  sidebar_layout->addSpacing(45);
  sidebar_layout->addWidget(close_btn, 0, Qt::AlignCenter);
  QObject::connect(close_btn, &QPushButton::clicked, this, &SettingsWindow::closeSettings);

  // setup panels
  DevicePanel *device = new DevicePanel(this);
  QObject::connect(device, &DevicePanel::reviewTrainingGuide, this, &SettingsWindow::reviewTrainingGuide);
  QObject::connect(device, &DevicePanel::showDriverView, this, &SettingsWindow::showDriverView);

  QList<QPair<QString, QWidget *>> panels = {
    {"장치", device},
    {"네트워크", network_panel(this)},
    {"토글메뉴", new TogglesPanel(this)},
    {"소프트웨어", new SoftwarePanel(this)},
    {"커뮤니티", community_panel()},
  };

#ifdef ENABLE_MAPS
  auto map_panel = new MapPanel(this);
  panels.push_back({"Navigation", map_panel});
  QObject::connect(map_panel, &MapPanel::closeSettings, this, &SettingsWindow::closeSettings);
#endif

  const int padding = panels.size() > 3 ? 25 : 35;

  nav_btns = new QButtonGroup();
  for (auto &[name, panel] : panels) {
    QPushButton *btn = new QPushButton(name);
    btn->setCheckable(true);
    btn->setChecked(nav_btns->buttons().size() == 0);
    btn->setStyleSheet(QString(R"(
      QPushButton {
        color: grey;
        border: none;
        background: none;
        font-size: 60px;
        font-weight: 500;
        padding-top: %1px;
        padding-bottom: %1px;
      }
      QPushButton:checked {
        color: white;
      }
      QPushButton:pressed {
        color: #ADADAD;
      }
    )").arg(padding));

    nav_btns->addButton(btn);
    sidebar_layout->addWidget(btn, 0, Qt::AlignRight);

    const int lr_margin = name != "Network" ? 50 : 0;  // Network panel handles its own margins
    panel->setContentsMargins(lr_margin, 25, lr_margin, 25);

    ScrollView *panel_frame = new ScrollView(panel, this);
    panel_widget->addWidget(panel_frame);

    QObject::connect(btn, &QPushButton::clicked, [=, w = panel_frame]() {
      btn->setChecked(true);
      panel_widget->setCurrentWidget(w);
    });
  }
  sidebar_layout->setContentsMargins(50, 50, 100, 50);

  // main settings layout, sidebar + main panel
  QHBoxLayout *main_layout = new QHBoxLayout(this);

  sidebar_widget->setFixedWidth(500);
  main_layout->addWidget(sidebar_widget);
  main_layout->addWidget(panel_widget);

  setStyleSheet(R"(
    * {
      color: white;
      font-size: 50px;
    }
    SettingsWindow {
      background-color: black;
    }
  )");
}

void SettingsWindow::hideEvent(QHideEvent *event) {
#ifdef QCOM
  HardwareEon::close_activities();
#endif
}
