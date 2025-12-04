from django.urls import path
from .views import home, funcionario_create, funcionario_update, funcionario_delete

app_name = 'painel_admin'

urlpatterns = [
    path('', home, name='home'),
    path('funcionarios/novo/', funcionario_create, name='funcionario_create'),
    path('funcionarios/<int:pk>/editar/', funcionario_update, name='funcionario_update'),
    path('funcionarios/<int:pk>/excluir/', funcionario_delete, name='funcionario_delete'),
]