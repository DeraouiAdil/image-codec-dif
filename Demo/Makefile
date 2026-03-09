CODEC_DIR = ../CoDec
INPUT_IMG_DIR = ../img
INPUT_DIF_DIR = ../dif

.PHONY: all prepare clean help run_img run_dif

all: help

prepare:
	$(MAKE) -C $(CODEC_DIR) lib

run_img: prepare
	@echo "--- Lancement Scenario 1 (Image -> DIF -> PNM) ---"
	python3 demo.py $(INPUT_IMG_DIR) encode out_dif
	python3 demo.py out_dif/ decode out_pnm
	@echo "Terminé. Vérifiez le dossier out_pnm."

run_dif: prepare
	@echo "--- Lancement Scenario 2 (DIF -> PNM) ---"
	python3 demo.py $(INPUT_DIF_DIR) decode out_from_dif
	@echo "Terminé. Vérifiez le dossier out_from_dif."

clean:
	rm -rf out_dif out_pnm out_from_dif __pycache__

help:
	@echo "Commandes disponibles :"
	@echo "  make run_img   : Encode les images de $(INPUT_IMG_DIR) puis décode."
	@echo "  make run_dif   : Décode les fichiers DIF de $(INPUT_DIF_DIR)."
	@echo "  make clean     : Nettoie les fichiers générés."