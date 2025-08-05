# 🔒 Guide de Protection de Branche GitHub

## Configuration pour esphome-heishamon

### 📋 **Structure de Branches**

```
main           ← 🔒 BRANCHE PRINCIPALE (à protéger)
├── dev        ← Développement actif
├── feature/*  ← Nouvelles fonctionnalités  
└── hotfix/*   ← Corrections urgentes
```

### 🛡️ **Protection de la Branche `main`**

#### Accès Configuration
1. **GitHub** → Repository `esphome-heishamon` 
2. **Settings** → **Branches**
3. **Add rule** → Branch name pattern: `main`

#### ⚙️ **Règles de Protection Recommandées**

```yaml
Branch Protection Settings for 'main':

✅ Require a pull request before merging
  ├── Required number of reviewers: 1
  ├── Dismiss stale PR approvals when new commits are pushed
  ├── Require review from code owners (@AntorFr)
  └── Restrict pushes that create files larger than 100 MB

✅ Require status checks to pass before merging  
  ├── Require branches to be up to date before merging
  ├── Status checks required:
  │   ├── "ESPHome ESP32 Validation" 
  │   ├── "ESPHome ESP8266 Validation"
  │   └── "Code Quality Check"

✅ Require conversation resolution before merging

✅ Restrict pushes that create files larger than 100 MB

✅ Restrict force pushes ⚠️ CRITIQUE
  └── Empêche git push --force sur main

❌ Allow deletions  
  └── Interdit la suppression de la branche main

✅ Do not allow bypassing the above settings
  └── Même les admins doivent suivre les règles
```

### 🚀 **Configuration GitHub Actions (CI/CD)**

Le repository contient déjà les workflows dans `.github/workflows/` :

#### `ci.yml` - Validation Continue
```yaml
- ESPHome compilation validation (ESP32/ESP8266)
- Code quality checks
- Branch protection enforcement
```

#### `release.yml` - Gestion des Releases  
```yaml
- Automated releases from main
- Semantic versioning
- Release notes generation
```

#### `branch-protection.yml` - Protection Automatique
```yaml
- Enforce branch protection rules
- Status check requirements
- PR validation
```

### 📝 **Workflow de Développement**

#### Développement Standard
```bash
# 1. Créer une branche feature depuis dev
git checkout dev
git pull origin dev
git checkout -b feature/nouvelle-fonctionnalite

# 2. Développer et tester
# ... développement ...
./test-compilation.sh

# 3. Push et créer PR vers dev
git push origin feature/nouvelle-fonctionnalite
# → Créer PR: feature/xxx → dev

# 4. Après validation, merger dans dev
# 5. Périodiquement: PR dev → main pour releases
```

#### Releases Stables
```bash
# Uniquement via Pull Request dev → main
# ✅ Validation automatique ESPHome
# ✅ Review code mandatory  
# ✅ All status checks passed
# ✅ No force push allowed
```

### 🔍 **Status Checks Requis**

Les checks suivants doivent passer avant merge vers `main` :

```yaml
Required Status Checks:
- ✅ ESPHome ESP32 validation
- ✅ ESPHome ESP8266 validation  
- ✅ Code quality scan
- ✅ No TODO/FIXME in production code
- ✅ Documentation updated
- ✅ CHANGELOG.md updated (for releases)
```

### 👥 **Code Owners (CODEOWNERS)**

Créer `.github/CODEOWNERS` :
```bash
# Global ownership
* @AntorFr

# Component core
/components/heishamon/ @AntorFr

# Documentation  
/docs/ @AntorFr

# Configuration examples
example*.yaml @AntorFr

# GitHub workflows
/.github/ @AntorFr
```

### 🆘 **Exceptions d'Urgence**

En cas d'urgence critique :

1. **Hotfix depuis main** :
   ```bash
   git checkout main
   git checkout -b hotfix/fix-critique
   # ... correction ...
   # PR hotfix/xxx → main (fast-track review)
   ```

2. **Bypass temporaire** (admin uniquement) :
   - Désactiver temporairement protection
   - Appliquer fix direct
   - Réactiver protection immédiatement
   - Documenter l'incident

### 📊 **Monitoring Protection**

Surveillance de la protection de branche :

- **GitHub Insights** → Security → Branch protection rules
- **Audit log** pour les tentatives de bypass
- **PR metrics** : temps de review, taux d'approbation
- **CI/CD success rate** : validation automatique

---

## 🎯 **Action Immédiate**

**ÉTAPES À SUIVRE MAINTENANT** :

1. ✅ Branche `main` créée et synchronisée
2. ⏳ **Configurer protection sur GitHub** (manuel)
3. ⏳ Activer les GitHub Actions existantes  
4. ⏳ Définir les code owners
5. ⏳ Tester le workflow avec une PR dev → main

**La branche `main` est maintenant prête à être protégée ! 🔒**
