typedef struct {
  char* slova;
  int kapacitet;
  int velicina;
} MojaStringSekcija;

MojaStringSekcija *init_moja_string_sekcija();

int dodaj_string(MojaStringSekcija*, const char* novi);

int dodaj_string_povezano(MojaStringSekcija* ss, const char* prvi, const char* drugi);

void obrisi_moju_string_sekciju(MojaStringSekcija*);