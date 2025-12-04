from django.db import models

class Funcionario(models.Model):
    user_id = models.AutoField(primary_key=True)   # SERIAL
    name = models.TextField()
    rfid = models.TextField(null=True, blank=True)

    class Meta:
        db_table = 'users'  # public.funcionarios
        managed = False            # não criar/alterar por migrations

    def __str__(self):
        return f"{self.name} ({self.rfid or 'sem RFID'})"