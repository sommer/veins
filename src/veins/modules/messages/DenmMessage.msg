import veins.modules.messages.BaseFrame1609_4;

namespace veins;

packet DenmMessage extends BaseFrame1609_4 {
    uint32 stationId;           // ID della stazione che genera il DENM
    int16 causeCode;            // ETSI CauseCode (es. incidente, lavori, ...)
    int16 subCause;             // SubCause (se applicabile)
    uint32 validityDurationMs;  // Durata di validità dell'evento in ms (0 = non specificata)
    uint8 vanetzaPayload[];     // Payload ASN.1 (blob binario prodotto da Vanetza)
}