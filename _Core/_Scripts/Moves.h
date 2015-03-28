void createNormalMove(struct Board *, int *, int* , int *, int);
void createCastleMove(struct Board *, int *, int* , int *, int);
void createPromotionMove(struct Board *, int *, int* , int *, int);
void createEnpassMove(struct Board *, int *, int* , int *, int);

void applyGenericMove(struct Board *, int *);
void applyNormalMove(struct Board *, int *);
void applyCastleMove(struct Board *, int *);
void applyPromotionMove(struct Board *, int *);
void applyEnpassMove(struct Board *, int *);

void revertGenericMove(struct Board *, int *);
void revertNormalMove(struct Board *, int *);
void revertCastleMove(struct Board *, int *);
void revertPromotionMove(struct Board *, int *);
void revertEnpassMove(struct Board *, int *);