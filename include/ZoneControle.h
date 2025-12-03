// ZoneControle.h (Classe de base - optionnelle si vous voulez de l'héritage)
#ifndef ZONE_CONTROLE_H
#define ZONE_CONTROLE_H

#include <string>
#include "Position3D.h"

enum class TypeZone {
    CCR,    // Centre de Contrôle Régional
    APP,    // Approche
    TWR     // Tour de contrôle
};

// Classe de base abstraite (optionnelle)
class ZoneControle {
protected:
    std::string identifiant;
    TypeZone type;
    Position3D centre;
    double rayon;

public:
    ZoneControle(const std::string& id, TypeZone type, const Position3D& centre, double rayon)
        : identifiant(id), type(type), centre(centre), rayon(rayon) {
    }

    virtual ~ZoneControle() = default;

    // Méthodes communes
    virtual bool contientPosition(const Position3D& pos) const = 0;

    TypeZone getType() const { return type; }
    std::string getIdentifiant() const { return identifiant; }
    Position3D getCentre() const { return centre; }
    double getRayon() const { return rayon; }
};

#endif