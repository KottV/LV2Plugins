all: build generate_ttl

build: common \
	lv2cvport \
	SevenDelay \
	SyncSawSynth \
	WaveCymbal \
	FDNCymbal \
	TrapezoidSynth \
	IterativeSinCluster \
	EnvelopedSine \
	EsPhaser \
	CubicPadSynth \
	ModuloShaper \
	FoldShaper \

.PHONY: generate_ttl
generate_ttl: build
	./generate-ttl.sh

# DEBUG=true
# export DEBUG

LV2 ?= true
VST2 ?= true
JACK ?= true

.PHONY: dpf
dpf:
	$(MAKE) -C lib/DPF dgl utils/lv2_ttl_generator

.PHONY: common
common: dpf
	$(MAKE) -C common

.PHONY: lv2cvport
lv2cvport: common
	$(MAKE) -C lv2cvport

.PHONY: FoldShaper
FoldShaper: common
	$(MAKE) -C FoldShaper LV2=$(LV2) VST2=$(VST2) JACK=$(JACK)

.PHONY: ModuloShaper
ModuloShaper: common
	$(MAKE) -C ModuloShaper LV2=$(LV2) VST2=$(VST2) JACK=$(JACK)

.PHONY: CubicPadSynth
CubicPadSynth: common
	$(MAKE) -C CubicPadSynth LV2=$(LV2) VST2=$(VST2) JACK=$(JACK)

.PHONY: EsPhaser
EsPhaser: common
	$(MAKE) -C EsPhaser LV2=$(LV2) VST2=$(VST2) JACK=$(JACK)

.PHONY: EnvelopedSine
EnvelopedSine: common
	$(MAKE) -C EnvelopedSine LV2=$(LV2) VST2=$(VST2) JACK=$(JACK)

.PHONY: IterativeSinCluster
IterativeSinCluster: common
	$(MAKE) -C IterativeSinCluster LV2=$(LV2) VST2=$(VST2) JACK=$(JACK)

.PHONY: TrapezoidSynth
TrapezoidSynth: common
	$(MAKE) -C TrapezoidSynth LV2=$(LV2) VST2=$(VST2) JACK=$(JACK)

.PHONY: FDNCymbal
FDNCymbal: common
	$(MAKE) -C FDNCymbal LV2=$(LV2) VST2=$(VST2) JACK=$(JACK)

.PHONY: WaveCymbal
WaveCymbal: common
	$(MAKE) -C WaveCymbal LV2=$(LV2) VST2=$(VST2) JACK=$(JACK)

.PHONY: SyncSawSynth
SyncSawSynth: common
	$(MAKE) -C SyncSawSynth LV2=$(LV2) VST2=$(VST2) JACK=$(JACK)

.PHONY: SevenDelay
SevenDelay: common
	$(MAKE) -C SevenDelay LV2=$(LV2) VST2=$(VST2) JACK=$(JACK)

.PHONY: install
install:
	cp -r bin/*.lv2 ~/.lv2/

.PHONY: clean
clean:
	rm -r bin
	rm -r build

.PHONY: cleanDPF
cleanDPF:
	$(MAKE) -C lib/DPF clean

.PHONY: cleanall
cleanall: cleanDPF clean
