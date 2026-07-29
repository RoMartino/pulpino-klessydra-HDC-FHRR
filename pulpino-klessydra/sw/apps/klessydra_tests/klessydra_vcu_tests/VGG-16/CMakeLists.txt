# # add_application(vgg_zeropadded Vgg_Fully_ZEROPADDED.c LABELS "klessydra_tests")
# add_application(vgg_zeropadded_lite prova.c LABELS "klessydra_tests")


# # TESTING layers: tutte le uscite, test fatto per le stime per abdallah
# add_application(vgg_zeropadded_lite_all_outputs Vgg_Fully_ZEROPADDED_all_outputs.c LABELS "klessydra_tests")


# # TESTING layers: solo parte delle uscite, per il quick debug, con intercept attivo
# add_application(vgg_zeropadded_lite_quick_debug Vgg_Fully_ZEROPADDED_quick_debug.c LABELS "klessydra_tests")


# # TUTTI i layers: per fare tutti i layer, tutte le uscite
# add_application(vgg_zeropadded_tutta Vgg_prova_tutta.c LABELS "klessydra_tests")


# # TUTTI utti i layer ma non fa computazioni, è usata solo per il conteggio delle operazioni kmemld e kmemst delle FM e delle KM
# add_application(vgg_conto_kmem_1 Vgg_conto_kmem.c LABELS "klessydra_tests")


# # TUTTI i layer ma non fa computazioni, è usata solo per il conteggio delle operazioni kmemld e kmemst delle KM
# add_application(vgg_conto_kmem_2 Vgg_conto_kmem_2.c LABELS "klessydra_tests")


########################
# add_application(vgg_zeropadded_2 Vgg_2.c LABELS "klessydra_tests")

# add_application(vgg_zeropadded_5 Vgg_5.c LABELS "klessydra_tests")

# add_application(vgg_zeropadded_only_conv Vgg_only_conv.c LABELS "klessydra_tests")

# add_application(vgg_zeropadded_only_fc Vgg_only_fc.c LABELS "klessydra_tests")

# add_application(vgg_zeropadded_only_fc_singolo Vgg_only_fc_singolo.c LABELS "klessydra_tests")

# add_application(vgg_zeropadded_dicembre Vgg_dicembre.c LABELS "klessydra_tests")
# add_application(vgg_zeropadded_0_to_3 Vgg_dicembre.c LABELS "klessydra_tests")
# add_application(vgg_zeropadded_0_to_10 Vgg_dicembre.c LABELS "klessydra_tests")
# add_application(vgg_zeropadded_4_to_10 Vgg_dicembre.c LABELS "klessydra_tests")

add_application(VGG16_test VGG-16.c LABELS "klessydra_tests")
# add_application(vgg_zeropadded_22 Vgg_dicembre.c LABELS "klessydra_tests")
