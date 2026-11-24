#pragma once

#include <Geode/Geode.hpp>
#include <string>

namespace neo::updater {

class NeoUpdater {
public:
    // Comprueba si hay una nueva versión en GitHub.
    static void checkForUpdates(bool force = true);

private:
    // Muestra la alerta de actualización.
    static void showUpdate(
        const std::string& version,
        const std::string& downloadURL
    );

    // Descarga el paquete .geode al directorio del mod.
    static void downloadUpdate(
        const std::string& version,
        const std::string& downloadURL
    );

    // Muestra el resultado de la descarga.
    static void showDownloadResult(
        const std::string& version,
        const std::string& filePath
    );
};

}
