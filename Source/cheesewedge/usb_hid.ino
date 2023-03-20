void initiateHID(){
  usb_hid.setStringDescriptor("CheeseWedge Keyboard");
  usb_hid.begin();
}