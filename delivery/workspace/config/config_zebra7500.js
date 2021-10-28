
function setup()
{
	return {
	admin_pass: "caravane",
	devices: [
    {
        name: "nfc_reader", // Ne pas changer
        type: "Zebra7500", // Ne pas changer
        conn_channel: "192.168.0.49", // Changez en fonction de l'adresse IP du Zebra
        conn_settings: "",  // Ne pas changer
        id: "",   // Ne pas changer
        options: ""  // Ne pas changer
    }
    ]};
}

