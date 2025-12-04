from django import forms
from .models import Funcionario

class FuncionarioForm(forms.ModelForm):
    class Meta:
        model = Funcionario
        fields = ['name']  # nome → name
        widgets = {
            'name': forms.TextInput(attrs={  # nome → name
                'class': 'input',
                'placeholder': 'Nome completo'
            })
        }