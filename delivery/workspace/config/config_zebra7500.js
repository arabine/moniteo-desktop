function setup()
        {
            return {
            admin_pass: "caravane",
            devices: [
            {
                name: "nfc_reader", // Ne pas changer
                type: "Zebra7500", // Ne pas changer
                conn_channel: "192.168.1.1",
                conn_settings: "",  // Ne pas changer
                id: "",   // Ne pas changer
                options: ""  // Ne pas changer
            }
            ]};
        }
