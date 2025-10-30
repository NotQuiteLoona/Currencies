add_definitions(-DTRANSLATION_DOMAIN=\"plasma_runner_org.kde.currencies\")

kcoreaddons_add_plugin(currencies INSTALL_NAMESPACE "kf6/krunner")

target_sources(currencies PRIVATE
        currencies.cpp
)

target_link_libraries(currencies
    Qt6::Gui
    Qt6::Network
    KF6::Runner
    KF6::I18n
    KF6::ConfigCore
)
