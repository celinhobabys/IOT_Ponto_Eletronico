import paho.mqtt.client as mqtt
import json
from datetime import datetime
import os
from pathlib import Path
from dotenv import load_dotenv

BASE_DIR = Path(__file__).resolve().parent.parent
load_dotenv(BASE_DIR / '.env')

# Configurações MQTT
BROKER = os.getenv("MQTT_BROKER")
PORT = 1883
USERNAME = os.getenv("MQTT_USERNAME")
PASSWORD = os.getenv("MQTT_PASSWORD")
TOPIC = "ponto/cadastro/iniciar"

def publish_add_user():
    """Publica mensagem MQTT para adicionar usuário"""
    try:
        client = mqtt.Client()
        
        # Configurar credenciais
        client.username_pw_set(USERNAME, PASSWORD)
        
        # Conectar
        client.connect(BROKER, PORT, 60)
        
        # Mensagem
        message = "true"
        
        # Publicar
        result = client.publish(TOPIC, message)
        
        # Aguardar confirmação
        result.wait_for_publish()
        
        client.disconnect()
        
        return True
    except Exception as e:
        print(f"Erro MQTT: {e}")
        return False