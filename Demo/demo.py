import os
import sys
import ctypes
import glob

# 1. Chargement de la librairie C
def load_lib():
    lib_path = "../CoDec/bin/libcodec.so"
    if not os.path.exists(lib_path):
        print(f"Erreur: Impossible de trouver {lib_path}")
        print("Avez-vous fait 'make lib' dans le dossier CoDec ?")
        sys.exit(1)
    
    lib = ctypes.CDLL(lib_path)
    lib.pnmtodif.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
    lib.pnmtodif.restype = ctypes.c_int
    lib.diftopnm.argtypes = [ctypes.c_char_p, ctypes.c_char_p]
    lib.diftopnm.restype = ctypes.c_int
    return lib

# 2. Traitement d'un fichier unique
def process_file(lib, func, file_in, dir_out, ext_out, show_img):
    nom_fichier = os.path.basename(file_in)
    nom_sans_ext = os.path.splitext(nom_fichier)[0]
    file_out = os.path.join(dir_out, f"{nom_sans_ext}.{ext_out}")
    
    print(f"{nom_fichier:30} -> {ext_out.upper()} ... ", end="")
    sys.stdout.flush()
    
    # Appel de la fonction C
    res = func(file_in.encode('utf-8'), file_out.encode('utf-8'))
    
    if res == 0:
        print("\033[92mOK\033[0m") # Vert
        
        # --- GESTION DE L'AFFICHAGE ---
        # On affiche seulement si c'est demandé ET que c'est une image (pnm)
        if show_img and ext_out == "pnm":
            # On cherche le visualiseur (variable d'env ou 'display' par défaut)
            viewer = os.getenv("CODEC_VIEWER", "display")
            
            # On lance la commande en arrière-plan (&) pour ne pas bloquer le script
            cmd = f"{viewer} {file_out} &"
            os.system(cmd)
            
    else:
        print("\033[91mECHEC\033[0m") # Rouge

# 3. Récupération des fichiers
def get_all_files(inputs, mode):
    files = []
    
    for path in inputs:
        if os.path.isfile(path):
            files.append(path)
        elif os.path.isdir(path):
            if mode == 'decode':
                files.extend(glob.glob(os.path.join(path, "*.dif")))
            else:
                exts = ["pgm", "ppm", "pnm", "jpg", "jpeg", "png", "gif", 
                        "tif", "tiff", "webp", "heic", "heif"]
                for ext in exts:
                  files.extend(glob.glob(os.path.join(path, f"*.{ext}")))
                  files.extend(glob.glob(os.path.join(path, f"*.{ext.upper()}")))
        else:
          print(f"Attention : '{path}' introuvable ou ignoré.")

    return sorted(list(set(files)))

# 4. Fonction principale
def run(lib, inputs, dir_out, mode, show_img):
    if not os.path.exists(dir_out):
        os.makedirs(dir_out)
        print(f"Dossier de sortie créé : {dir_out}")

    func = lib.pnmtodif if mode == 'encode' else lib.diftopnm
    ext_out = "dif" if mode == 'encode' else "pnm"
    
    all_files = get_all_files(inputs, mode)

    if not all_files:
        print(f"Aucun fichier valide trouvé pour le mode {mode}.")
        return

    print(f"--- Mode {mode.upper()} : {len(all_files)} fichier(s) à traiter ---")
    for f in all_files:
        process_file(lib, func, f, dir_out, ext_out, show_img)

if __name__ == "__main__":
    args = sys.argv[1:]
    
    # Vérification de l'option --show
    show_img = False
    if "--show" in args:
        show_img = True
        args.remove("--show") # On l'enlève pour ne pas gêner la suite
    
    # Détection du mode (encode ou decode)
    mode = None
    if "encode" in args:
        mode = "encode"
    elif "decode" in args:
        mode = "decode"
    
    if mode is None:
        print("Usage: python3 demo.py <fichiers...> <encode|decode> [dossier_sortie] [--show]")
        sys.exit(1)

    # Séparation des arguments
    idx = args.index(mode)
    inputs = args[:idx]
    output_arg = args[idx+1:]

    if not inputs:
        print("Erreur : Aucun fichier d'entrée spécifié.")
        sys.exit(1)
        
    d_out = output_arg[0] if output_arg else "resultats"

    lib = load_lib()
    run(lib, inputs, d_out, mode, show_img)
    print("\nTerminé !")