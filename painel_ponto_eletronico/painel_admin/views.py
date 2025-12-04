from django.shortcuts import render, redirect, get_object_or_404
from django.contrib.auth.decorators import login_required
from django.utils.decorators import method_decorator
from django.views.decorators.cache import never_cache
from django.views.generic import CreateView
from django.urls import reverse_lazy
from django.contrib import messages
from .models import Funcionario
from .forms import FuncionarioForm
from .mqtt_client import publish_add_user

@never_cache
@login_required
def home(request):
    funcionarios = Funcionario.objects.all().order_by('user_id')  # id → user_id

    q = (request.GET.get('q') or '').strip()
    if q:
        funcionarios = funcionarios.filter(name__icontains=q)  # nome → name

    ctx = {
        'funcionarios': funcionarios,
        'q': q,
        'total': funcionarios.count()
    }
    return render(request, 'painel_admin/home.html', ctx)

""""
@method_decorator(login_required, name='dispatch')
class FuncionarioCreateView(CreateView):
    model = Funcionario
    form_class = FuncionarioForm
    template_name = 'painel_admin/funcionario_form_novo.html'
    success_url = reverse_lazy('painel_admin:home')
"""

@login_required
def funcionario_create(request):
    """Agora publica MQTT em vez de mostrar formulário"""
    if publish_add_user():
        messages.success(request, 'Comando de adicionar usuário enviado via MQTT!')
    else:
        messages.error(request, 'Erro ao enviar comando MQTT.')
    
    return redirect('painel_admin:home')

@login_required
def funcionario_update(request, pk):
    funcionario = get_object_or_404(Funcionario, pk=pk)
    if request.method == 'POST':
        form = FuncionarioForm(request.POST, instance=funcionario)
        if form.is_valid():
            form.save()
            return redirect('painel_admin:home')
    else:
        form = FuncionarioForm(instance=funcionario)
    return render(request, 'painel_admin/funcionario_form.html', {
        'form': form,
        'object': funcionario,
        'titulo': 'Editar Funcionário',
        'botao': 'Salvar Alterações'
    })

@login_required
def funcionario_delete(request, pk):
    funcionario = get_object_or_404(Funcionario, pk=pk)
    if request.method == 'POST':
        funcionario.delete()
        return redirect('painel_admin:home')
    return render(request, 'painel_admin/funcionario_confirm_delete.html', {'funcionario': funcionario})