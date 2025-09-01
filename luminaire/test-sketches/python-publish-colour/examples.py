    # Example 1: Set all pixels to solid Red
    print("\nSetting all pixels to RED (FF0000)...")
    red_payload = create_solid_color_payload(255, 0, 0)
    client.publish(MQTT_TOPIC, red_payload, qos=0)
    time.sleep(2) # Wait a bit to see the effect

    # Example 2: Set all pixels to solid Green
    print("\nSetting all pixels to GREEN (00FF00)...")
    green_payload = create_solid_color_payload(0, 255, 0)
    client.publish(MQTT_TOPIC, green_payload, qos=0)
    time.sleep(2)

    # Example 3: Set all pixels to solid Blue
    print("\nSetting all pixels to BLUE (0000FF)...")
    blue_payload = create_solid_color_payload(0, 0, 255)
    client.publish(MQTT_TOPIC, blue_payload, qos=0)
    time.sleep(2)

    # Example 4: Set all pixels to White (dimmed)
    print("\nSetting all pixels to DIM WHITE (505050)...")
    dim_white_payload = create_solid_color_payload(80, 80, 80)
    client.publish(MQTT_TOPIC, dim_white_payload, qos=0)
    time.sleep(2)

    # Example 5: Set all pixels to OFF (Black)
    print("\nSetting all pixels to OFF (000000)...")
    off_payload = create_solid_color_payload(0, 0, 0)
    client.publish(MQTT_TOPIC, off_payload, qos=0)
    time.sleep(2)

    # Example 6: Random colors for each LED
    print("\nSetting random colors for each pixel (5 updates)...")
    for _ in range(5):
        random_payload = create_random_color_payload()
        client.publish(MQTT_TOPIC, random_payload, qos=0)
        time.sleep(0.5) # Faster updates for animation effect

    # Example 7: Green to Blue Gradient
    print("\nSetting a Green to Blue gradient...")
    gradient_payload = create_gradient_payload((0, 255, 0), (0, 0, 255))
    client.publish(MQTT_TOPIC, gradient_payload, qos=0)
    time.sleep(2)


    # Example 8: Green to Red Gradient fading in and out
    for n in range(10):
        
        print("\nSetting a Green to Blue gradient...")
        for i in range(50):
            gradient_payload = create_gradient_payload((0, i*5, 0), (i*5, 0, 0))
            client.publish(MQTT_TOPIC, gradient_payload, qos=0)
            time.sleep(0.1)
        for i in range(50):
            colour = 255-(i*5)
            gradient_payload = create_gradient_payload((0, colour, 0), (colour, 0, 0))
            client.publish(MQTT_TOPIC, gradient_payload, qos=0)
            time.sleep(0.1)


    # Example 6: Random colors for each LED
    print("\nSetting random red for each pixel (5 updates)...")
    for _ in range(5):
        random_payload = create_random_red_payload()
        client.publish(MQTT_TOPIC, random_payload, qos=0)
        time.sleep(0.5) # Faster updates for animation effect
